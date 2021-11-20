/*
 * SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "jwsverifier_p.h"
#include "jsonld_p.h"
#include "logging.h"
#include "rdf_p.h"

#include <QFile>
#include <QJsonDocument>

#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/pem.h>

JwsVerifier::JwsVerifier(const QJsonObject &doc)
    : m_obj(doc)
{
}

JwsVerifier::~JwsVerifier() = default;

bool JwsVerifier::verify() const
{
    const auto proof = m_obj.value(QLatin1String("proof")).toObject();
    const auto jws = proof.value(QLatin1String("jws")).toString();

    // see RFC 7515 §3.1. JWS Compact Serialization Overview
    const auto payloadStart = jws.indexOf(QLatin1Char('.'));
    if (payloadStart < 0) {
        return false;
    }
    const auto header = QStringView(jws).left(payloadStart);
    const auto sigStart = jws.indexOf(QLatin1Char('.'), payloadStart + 1);
    if (sigStart < 0) {
        return false;
    }
    //const auto payload = QStringView(jws).mid(payloadStart + 1, sigStart - payloadStart - 1);
    const auto signature = QByteArray::fromBase64(QStringView(jws).mid(sigStart + 1).toUtf8(), QByteArray::Base64UrlEncoding);

    // check signature algorithm
    const auto headerObj = QJsonDocument::fromJson(QByteArray::fromBase64(header.toUtf8(), QByteArray::Base64UrlEncoding)).object();
    if (headerObj.value(QLatin1String("alg")) != QLatin1String("PS256")) {
        qCWarning(Log) << "not implemented JWS algorithm:" << headerObj;
        return false;
    }

    // load certificate
    const auto evp = loadPublicKey();
    if (!evp) {
        return false;
    }

    const EVP_MD *digest = EVP_sha256();
    uint8_t digestData[EVP_MAX_MD_SIZE];
    uint32_t  digestSize = 0;

    // prepare the canonicalized form of the signed content
    QJsonObject content = m_obj;
    QJsonObject proofOptions = content.take(QLatin1String("proof")).toObject();
    proofOptions.remove(QLatin1String("jws"));
    proofOptions.remove(QLatin1String("signatureValue"));
    proofOptions.remove(QLatin1String("proofValue"));
    proofOptions.insert(QLatin1String("@context"), QLatin1String("https://w3id.org/security/v2"));

    const auto canonicalProof = canonicalRdf(proofOptions);
    const auto canonicalContent = canonicalRdf(content);

    QByteArray signedData = header.toUtf8() + '.';
    EVP_Digest(reinterpret_cast<const uint8_t*>(canonicalProof.constData()), canonicalProof.size(), digestData, &digestSize, digest, nullptr);
    signedData.append(reinterpret_cast<const char*>(digestData), digestSize);
    EVP_Digest(reinterpret_cast<const uint8_t*>(canonicalContent.constData()), canonicalContent.size(), digestData, &digestSize, digest, nullptr);
    signedData.append(reinterpret_cast<const char*>(digestData), digestSize);

    // compute hash of the signed data
    EVP_Digest(reinterpret_cast<const uint8_t*>(signedData.constData()), signedData.size(), digestData, &digestSize, digest, nullptr);

    // verify
    openssl::evp_pkey_ctx_ptr ctx(EVP_PKEY_CTX_new(evp.get(), nullptr), &EVP_PKEY_CTX_free);
    if (!ctx || EVP_PKEY_verify_init(ctx.get()) <= 0) {
        return false;
    }
    if (EVP_PKEY_CTX_set_rsa_padding(ctx.get(), RSA_PKCS1_PSS_PADDING) <= 0 || EVP_PKEY_CTX_set_signature_md(ctx.get(), digest) <= 0) {
        return false;
    }

    const auto verifyResult = EVP_PKEY_verify(ctx.get(), reinterpret_cast<const uint8_t*>(signature.constData()), signature.size(),  digestData, digestSize);
    switch (verifyResult) {
        case -1: // technical issue
            qCWarning(Log) << "Failed to verify signature:" << ERR_error_string(ERR_get_error(), nullptr);
            break;
        case 1: // valid signature;
            return true;
    }
    return false;
}

openssl::evp_pkey_ptr JwsVerifier::loadPublicKey() const
{
    // ### for now there is only one key, longer term we probably need to actually
    // implement finding the right key here
    openssl::evp_pkey_ptr evp(nullptr, &EVP_PKEY_free);

    QFile pemFile(QLatin1String(":/org.kde.khealthcertificate/divoc/did-india.pem"));
    if (!pemFile.open(QFile::ReadOnly)) {
        qCWarning(Log) << "unable to load public key file:" << pemFile.errorString();
        return evp;
    }

    const auto pemData = pemFile.readAll();
    const openssl::bio_ptr bio(BIO_new_mem_buf(pemData.constData(), pemData.size()), &BIO_free_all);
    openssl::rsa_ptr rsa(PEM_read_bio_RSA_PUBKEY(bio.get(), nullptr, nullptr, nullptr), &RSA_free);
    if (!rsa) {
        qCWarning(Log) << "Failed to read public key." << ERR_error_string(ERR_get_error(), nullptr);
        return evp;
    }

    evp.reset(EVP_PKEY_new());
    EVP_PKEY_assign_RSA(evp.get(), rsa.release());
    return evp;
}

static struct {
    const char *uri;
    const char *filePath;
} constexpr const schema_document_table[] = {
    { "https://www.w3.org/2018/credentials/v1", ":/org.kde.khealthcertificate/divoc/credentials-v1.json" },
    { "https://cowin.gov.in/credentials/vaccination/v1", ":/org.kde.khealthcertificate/divoc/vaccination-v1.json" },
    { "https://w3id.org/security/v1", ":/org.kde.khealthcertificate/divoc/security-v1.json" },
    { "https://w3id.org/security/v2", ":/org.kde.khealthcertificate/divoc/security-v2.json" },
};

QByteArray JwsVerifier::canonicalRdf(const QJsonObject &doc) const
{
    JsonLd jsonLd;
    const auto documentLoader = [](const QString &context) -> QByteArray {
        for (const auto &i : schema_document_table) {
            if (context == QLatin1String(i.uri)) {
                QFile f(QLatin1String(i.filePath));
                if (!f.open(QFile::ReadOnly)) {
                    qCWarning(Log) << f.errorString();
                } else {
                    return f.readAll();
                }
            }
        }
        qCWarning(Log) << "Failed to provide requested document:" << context;
        return QByteArray();
    };
    jsonLd.setDocumentLoader(documentLoader);

    auto quads = jsonLd.toRdf(doc);
    Rdf::normalize(quads);
    return Rdf::serialize(quads);
}

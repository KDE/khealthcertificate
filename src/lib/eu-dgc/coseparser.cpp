/*
 * SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "coseparser_p.h"
#include "cborutils_p.h"
#include "logging.h"

#include <openssl/verify_p.h>
#include <openssl/x509loader_p.h>

#include <QCborMap>
#include <QCborStreamReader>
#include <QCborStreamWriter>
#include <QCborValue>
#include <QFile>

#include <openssl/bn.h>
#include <openssl/engine.h>
#include <openssl/err.h>
#include <openssl/pem.h>

enum {
    CoseHeaderAlgorithm = 1,
    CoseHeaderKid = 4
};

enum {
    CoseAlgorithmECDSA_SHA256 = -7,
    CoseAlgorithmECDSA_SHA384 = -35,
    CoseAlgorithmECDSA_SHA512 = -36,
    CoseAlgorithmRSA_PSS_256 = -37,
    CoseAlgorithmRSA_PSS_384 = -38,
    CoseAlgorithmRSA_PSS_512 = -39,
};

void CoseParser::parse(const QByteArray &data)
{
    clear();

    // only single signer case implemented atm
    QCborStreamReader reader(data);
    if (reader.type() != QCborStreamReader::Tag || reader.toTag() != QCborKnownTags::COSE_Sign1) {
        qCWarning(Log) << "wrong COSE tag:" << reader.toTag();
        return;
    }

    reader.next();
    if (!reader.isArray()) {
        return;
    }

    reader.enterContainer();
    m_protectedParams = CborUtils::readByteArray(reader);
    auto params = QCborValue::fromCbor(m_protectedParams);
    const auto algorithm = params.toMap().value(CoseHeaderAlgorithm).toInteger();
    m_kid = params.toMap().value(CoseHeaderKid).toByteArray();
    params = QCborValue::fromCbor(reader);
    if (m_kid.isEmpty()) {
        m_kid = params.toMap().value(CoseHeaderKid).toByteArray();
    }
    m_payload = CborUtils::readByteArray(reader);
    m_signature = CborUtils::readByteArray(reader);

    // find and load certificate
    QFile certFile(QLatin1String(":/org.kde.khealthcertificate/eu-dgc/certs/") + QString::fromUtf8(m_kid.toHex()) + QLatin1String(".der"));
    if (!certFile.open(QFile::ReadOnly)) {
        qCWarning(Log) << "unable to find certificate for key id:" << m_kid.toHex();
        m_signatureState = UnknownCertificate;
        return;
    }

    const auto certData = certFile.readAll();
    const auto cert = X509Loader::readFromDER(certData);
    if (!cert) {
        qCWarning(Log) << "failed to read X509 certificate";
        m_signatureState = UnknownCertificate;
        return;
    }
    const openssl::evp_pkey_ptr pkey(X509_get_pubkey(cert.get()), &EVP_PKEY_free);
    if (!pkey) {
        qCWarning(Log) << "failed to load public key";
        m_signatureState = UnknownCertificate;
        return;
    }
    m_certificate = QSslCertificate(certData, QSsl::Der);

    switch (algorithm) {
        case CoseAlgorithmECDSA_SHA256:
        case CoseAlgorithmECDSA_SHA384:
        case CoseAlgorithmECDSA_SHA512:
            validateECDSA(pkey, algorithm);
            break;
        case CoseAlgorithmRSA_PSS_256:
        case CoseAlgorithmRSA_PSS_384:
        case CoseAlgorithmRSA_PSS_512:
            validateRSAPSS(pkey, algorithm);
            break;
        default:
            qCWarning(Log) << "signature algorithm not implemented yet:" << algorithm;
            m_signatureState = UnsupportedAlgorithm;
            return;
    }
}

QByteArray CoseParser::payload() const
{
    return m_payload;
}

CoseParser::SignatureState CoseParser::signatureState() const
{
    return m_signatureState;
}

QSslCertificate CoseParser::certificate() const
{
    return m_certificate;
}

void CoseParser::clear()
{
    m_protectedParams.clear();
    m_payload.clear();
    m_signature.clear();
    m_kid.clear();
    m_signatureState = Unknown;
}

void CoseParser::validateECDSA(const openssl::evp_pkey_ptr &pkey, int algorithm)
{
    const openssl::ec_key_ptr ecKey(EVP_PKEY_get1_EC_KEY(pkey.get()), &EC_KEY_free);

    const EVP_MD *digest = nullptr;
    switch (algorithm) {
        case CoseAlgorithmECDSA_SHA256:
            digest = EVP_sha256();
            break;
        case CoseAlgorithmECDSA_SHA384:
            digest = EVP_sha384();
            break;
        case CoseAlgorithmECDSA_SHA512:
            digest = EVP_sha512();
            break;
    }

    // compute hash of the signed data
    const auto signedData = sigStructure();
    m_signatureState = Verify::verifyECDSA(pkey, digest, signedData.constData(), signedData.size(), m_signature.constData(), m_signature.size()) ?
        ValidSignature : InvalidSignature;
}

void CoseParser::validateRSAPSS(const openssl::evp_pkey_ptr &pkey, int algorithm)
{
    // compute hash of the signed data
    const EVP_MD *digest = nullptr;
    switch (algorithm) {
        case CoseAlgorithmRSA_PSS_256:
            digest = EVP_sha256();
            break;
        case CoseAlgorithmRSA_PSS_384:
            digest = EVP_sha384();
            break;
        case CoseAlgorithmRSA_PSS_512:
            digest = EVP_sha512();
            break;
    }

    const auto signedData = sigStructure();
    uint8_t digestData[EVP_MAX_MD_SIZE];
    uint32_t  digestSize = 0;
    EVP_Digest(reinterpret_cast<const uint8_t*>(signedData.constData()), signedData.size(), digestData, &digestSize, digest, nullptr);

    // verify
    openssl::evp_pkey_ctx_ptr ctx(EVP_PKEY_CTX_new(pkey.get(), nullptr), &EVP_PKEY_CTX_free);
    if (!ctx || EVP_PKEY_verify_init(ctx.get()) <= 0) {
        return;
    }
    if (EVP_PKEY_CTX_set_rsa_padding(ctx.get(), RSA_PKCS1_PSS_PADDING) <= 0 || EVP_PKEY_CTX_set_signature_md(ctx.get(), digest) <= 0) {
        return;
    }

    const auto verifyResult = EVP_PKEY_verify(ctx.get(), reinterpret_cast<const uint8_t*>(m_signature.constData()), m_signature.size(),  digestData, digestSize);
    switch (verifyResult) {
        case -1: // technical issue
            m_signatureState = InvalidSignature;
            qCWarning(Log) << "Failed to verify signature:" << ERR_error_string(ERR_get_error(), nullptr);
            break;
        case 0: // invalid signature
            m_signatureState = InvalidSignature;
            break;
        case 1: // valid signature;
            m_signatureState = ValidSignature;
            break;
    }
}

QByteArray CoseParser::sigStructure() const
{
    QByteArray data;
    {
        QCborStreamWriter writer(&data);
        writer.startArray(4);
        writer.append(QLatin1String("Signature1"));
        writer.append(m_protectedParams);
        writer.append(QByteArray());
        writer.append(m_payload);
        writer.endArray();
    }
    return data;
}

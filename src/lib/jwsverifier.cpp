/*
 * SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "jwsverifier_p.h"
#include "logging.h"

#include <QFile>
#include <QJsonDocument>

#include <openssl/err.h>
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
    const auto payload = QStringView(jws).mid(payloadStart + 1, sigStart - payloadStart - 1);
    const auto signature = QStringView(jws).mid(sigStart + 1);

    // check signature algorithm
    const auto headerObj = QJsonDocument::fromJson(QByteArray::fromBase64(header.toUtf8(), QByteArray::Base64UrlEncoding)).object();
    qDebug() << headerObj << payload << signature << QByteArray::fromBase64(signature.toUtf8(), QByteArray::Base64UrlEncoding).toHex();
    if (headerObj.value(QLatin1String("alg")) != QLatin1String("PS256")) {
        qCWarning(Log) << "not implemented JWS algorithm:" << headerObj;
        return false;
    }

    // load certificate
    const auto rsa = loadPublicKey();
    if (!rsa) {
        qCWarning(Log) << "Failed to read public key." << ERR_error_string(ERR_get_error(), nullptr);;
        return false;
    }

    // TODO
    return false;
}

openssl::rsa_ptr JwsVerifier::loadPublicKey() const
{
    // ### for now there is only one key, longer term we probably need to actually
    // implement finding the right key here
    openssl::rsa_ptr rsa(nullptr, &RSA_free);

    QFile pemFile(QLatin1String(":/org.kde.khealthcertificate/divoc/did-india.pem"));
    if (!pemFile.open(QFile::ReadOnly)) {
        qCWarning(Log) << "unable to load public key file:" << pemFile.errorString();
        return rsa;
    }

    const auto pemData = pemFile.readAll();
    const openssl::bio_ptr bio(BIO_new_mem_buf(pemData.constData(), pemData.size()), &BIO_free_all);

    rsa.reset(PEM_read_bio_RSA_PUBKEY(bio.get(), nullptr, nullptr, nullptr));
    return rsa;
}

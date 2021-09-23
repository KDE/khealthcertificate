/*
 * SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "jwtparser_p.h"
#include "jwkloader_p.h"
#include "logging.h"
#include "verify_p.h"
#include "zlib_p.h"

#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>

JwtParser::JwtParser() = default;
JwtParser::~JwtParser() = default;

void JwtParser::parse(const QByteArray &data)
{
    const auto idx1 = data.indexOf('.');
    if (idx1 < 0) {
        return;
    }
    const auto header = QJsonDocument::fromJson(QByteArray::fromBase64(data.left(idx1), QByteArray::Base64UrlEncoding)).object();

    const auto idx2 = data.indexOf('.', idx1 + 1);
    auto rawPayload = QByteArray::fromBase64(data.mid(idx1 + 1, idx2 - idx1 - 1), QByteArray::Base64UrlEncoding);
    if (header.value(QLatin1String("zip")).toString() == QLatin1String("DEF")) {
        rawPayload = Zlib::decompressDeflate(rawPayload);
    }
    m_payload = QJsonDocument::fromJson(rawPayload).object();

    // signature verification
    const auto signature = QByteArray::fromBase64(data.mid(idx2 + 1), QByteArray::Base64UrlEncoding);
    const auto kid = header.value(QLatin1String("kid")).toString();
    const auto evp = JwkLoader::loadPublicKey(QLatin1String(":/org.kde.khealthcertificate/shc/certs/") + kid + QLatin1String(".jwk"));
    if (!evp) {
        qCWarning(Log) << "no key found for kid:" << kid;
        return;
    }
    const auto alg = header.value(QLatin1String("alg")).toString();
    bool valid = false;
    if (alg == QLatin1String("ES256")) {
        valid = Verify::verifyECDSA(evp, EVP_sha256(), data.constData(), idx2, signature.constData(), signature.size());
    } else if (alg == QLatin1String("ES384")) {
        valid = Verify::verifyECDSA(evp, EVP_sha384(), data.constData(), idx2, signature.constData(), signature.size());
    } else if (alg == QLatin1String("ES512")) {
        valid = Verify::verifyECDSA(evp, EVP_sha512(), data.constData(), idx2, signature.constData(), signature.size());
    } else {
        qCWarning(Log) << "signature algorithm not supported:" << alg;
    }

    m_signatureState = valid ? KHealthCertificate::ValidSignature : KHealthCertificate::InvalidSignature;
}

QJsonObject JwtParser::payload() const
{
    return m_payload;
}

KHealthCertificate::SignatureValidation JwtParser::signatureState() const
{
    return m_signatureState;
}

/*
 * SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "jwsverifier_p.h"

#include <QDebug>
#include <QJsonDocument>

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

    const auto headerObj = QJsonDocument::fromJson(QByteArray::fromBase64(header.toUtf8(), QByteArray::Base64UrlEncoding)).object();
    qDebug() << headerObj << payload << signature << QByteArray::fromBase64(signature.toUtf8(), QByteArray::Base64UrlEncoding).toHex();

    // TODO
    return false;
}

/*
 * SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "jwtparser_p.h"
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
    qDebug() << header;

    const auto idx2 = data.indexOf('.', idx1 + 1);
    auto rawPayload = QByteArray::fromBase64(data.mid(idx1 + 1, idx2 - idx1 - 1), QByteArray::Base64UrlEncoding);
    if (header.value(QLatin1String("zip")).toString() == QLatin1String("DEF")) {
        rawPayload = Zlib::decompressDeflate(rawPayload);
    }
    m_payload = QJsonDocument::fromJson(rawPayload).object();

    // signature
    qDebug() << QByteArray::fromBase64(data.mid(idx2 + 1), QByteArray::Base64UrlEncoding).toHex();
    // TODO verify
}

QJsonObject JwtParser::payload() const
{
    return m_payload;
}

/*
 * SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "coseparser.h"
#include "cborutils_p.h"

#include <QCborMap>
#include <QCborStreamReader>
#include <QCborValue>
#include <QDebug>

void CoseParser::parse(const QByteArray &data)
{
    // only single signer case implemented atm
    QCborStreamReader reader(data);
    if (reader.type() != QCborStreamReader::Tag || reader.toTag() != QCborKnownTags::COSE_Sign1) {
        qDebug() << "wrong COSE tag:" << reader.toTag();
        return;
    }

    reader.next();
    if (!reader.isArray()) {
        return;
    }

    reader.enterContainer();
    m_protectedParams = CborUtils::readByteArray(reader);
    const auto algo = QCborValue::fromCbor(m_protectedParams);
    qDebug() << algo.toMap().value(1); // 1: algorithm
    if (reader.isMap()) { // unprotected params
        reader.enterContainer();
        while (reader.hasNext()) {
            const auto key = CborUtils::readInteger(reader);
            qDebug() << key << CborUtils::readByteArray(reader).toBase64(); // 4: key id, as found in dsc-list.json
        }
        reader.leaveContainer();
    }
    m_payload = CborUtils::readByteArray(reader);
    m_signature = CborUtils::readByteArray(reader);
}

QByteArray CoseParser::payload() const
{
    return m_payload;
}

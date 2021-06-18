/*
 * SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "coseparser.h"

#include <QCborStreamReader>
#include <QDebug>

void CoseParser::parse(const QByteArray &data)
{
    // only single signer case implemented atm
    QCborStreamReader reader(data);
    if (reader.toTag() != QCborKnownTags::COSE_Sign1) {
        qDebug() << "wrong COSE tag:" << reader.toTag();
        return;
    }

    reader.next();
    if (!reader.isArray()) {
        return;
    }

    reader.enterContainer();
    reader.next(); // algorithm?
    reader.next(); // key id?
    payload = reader.readByteArray().data; // TODO correct byte array reading
}

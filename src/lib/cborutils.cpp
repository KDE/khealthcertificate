/*
 * SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "cborutils_p.h"

#include <QCborStreamReader>
#include <QDebug>

QString CborUtils::readString(QCborStreamReader &reader)
{
    if (!reader.isString()) {
        return {};
    }

    QString result;
    auto r = reader.readString();
    while (r.status == QCborStreamReader::Ok) {
        result += r.data;
        r = reader.readString();
    }
    if (r.status == QCborStreamReader::Error) {
        qDebug() << "CBOR string read error";
        result.clear();
    }
    return result;
}

QByteArray CborUtils::readByteArray(QCborStreamReader &reader)
{
    if (!reader.isByteArray()) {
        return {};
    }

    QByteArray result;
    auto r = reader.readByteArray();
    while (r.status == QCborStreamReader::Ok) {
        result += r.data;
        r = reader.readByteArray();
    }

    if (r.status == QCborStreamReader::Error) {
        qDebug() << "CBOR byte array read error";
        result.clear();
    }
    return result;
}

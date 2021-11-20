/*
 * SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef CBORUTILS_P_H
#define CBORUTILS_P_H

#include <cstdint>

class QByteArray;
class QCborStreamReader;
class QString;

/** Utility functions for dealing with CBOR data. */
namespace CborUtils
{
    /** Read an integer value if that is the next type, and forward the reader. */
    int64_t readInteger(QCborStreamReader &reader);
    /** Read a fully assembled string value. */
    QString readString(QCborStreamReader &reader);
    /** Read a fully assembled byte array value. */
    QByteArray readByteArray(QCborStreamReader &reader);
}

#endif // CBORUTILS_P_H

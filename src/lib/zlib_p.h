/*
 * SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef ZLIB_P_H
#define ZLIB_P_H

class QByteArray;

/** Zlib convenience methods. */
namespace  Zlib
{
QByteArray decompressZlib(const QByteArray &data);
QByteArray decompressDeflate(const QByteArray &data);
}

#endif // ZLIB_P_H

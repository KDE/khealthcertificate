/*
 * SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef COSEPARSER_H
#define COSEPARSER_H

#include <QByteArray>

/** Parser for CBOR Object Signing and Encryption (COSE) data.
 *  @see RFC 8152
 */
class CoseParser
{
public:
    void parse(const QByteArray &data);

    QByteArray payload;
};

#endif // COSEPARSER_H

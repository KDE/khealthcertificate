/*
 * SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef VERIFIABLECREDENTIALPARSER_H
#define VERIFIABLECREDENTIALPARSER_H

class QByteArray;
class QVariant;

/** Parser for VerifiableCredential certificates, such as used in India. */
namespace VerifiableCredentialParser
{
    QVariant parse(const QByteArray &data);
}

#endif // VERIFIABLECREDENTIALPARSER_H

/*
 * SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef DIVOCPARSER_P_H
#define DIVOCPARSER_P_H

class QByteArray;
class QVariant;

/** Parser for DIVOC certificates, such as used in India.
 *  @see https://divoc.egov.org.in/
 */
namespace DivocParser
{
    QVariant parse(const QByteArray &data);
}

#endif // DIVOCPARSER_P_H

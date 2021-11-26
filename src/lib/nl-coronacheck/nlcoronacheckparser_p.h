/*
 * SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef NLCORONACHECKPARSER_H
#define NLCORONACHECKPARSER_H

class QByteArray;
class QVariant;

/** Parser for NL COVID-19 CoronaCheck codes. */
class NLCoronaCheckParser
{
public:
    static void init();
    static QVariant parse(const QByteArray &data);
};

#endif // NLCORONACHECKPARSER_H

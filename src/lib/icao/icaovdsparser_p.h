/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef ICAOVDSPARSER_H
#define ICAOVDSPARSER_H

class QByteArray;
class QVariant;

/** Parser foe ICAO VDS-NC certificats. */
class IcaoVdsParser
{
public:
    static void init();
    static QVariant parse(const QByteArray &data);
};

#endif // ICAOVDSPARSER_H

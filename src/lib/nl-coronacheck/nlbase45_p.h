/*
 * SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef NLBASE45_H
#define NLBASE45_H

class QByteArray;

/** NL COVID-19 CoronaCheck uses a modified Base45 encoding... */
namespace NLBase45
{
    QByteArray decode(const char *begin, const char *end);
};

#endif // NLBASE45_H

/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef SHCPARSER_P_H
#define SHCPARSER_P_H

class KVaccinationCertificate;

class QByteArray;
class QJsonObject;
class QVariant;

/** Parser for Smart Health Cards
 *  @see https://spec.smarthealth.cards/
 */
class ShcParser
{
public:
    static void init();
    static QVariant parse(const QByteArray &data);

private:
    static KVaccinationCertificate parseImmunization(const QJsonObject &obj);
};

#endif // SHCPARSER_P_H

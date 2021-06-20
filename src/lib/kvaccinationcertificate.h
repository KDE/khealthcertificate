/*
 * SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KVACCINATIONCERTIFICATE_H
#define KVACCINATIONCERTIFICATE_H

#include "khealthcertificate_export.h"
#include "khealthcertificatetypes.h"

#include <QDate>

class KVaccinationCertificatePrivate;

/** A vaccination certificate. */
class KHEALTHCERTIFICATE_EXPORT KVaccinationCertificate
{
    KHEALTHCERTIFICATE_GADGET(Vaccination)
    KHEALTHCERTIFICATE_PROPERTY(QString, name, setName)
    KHEALTHCERTIFICATE_PROPERTY(QDate, dateOfBirth, setDateOfBirth)
    KHEALTHCERTIFICATE_PROPERTY(QDate, date, setDate)
    KHEALTHCERTIFICATE_PROPERTY(QString, vaccine, setVaccine)
    KHEALTHCERTIFICATE_PROPERTY(QString, manufacturer, setManufacturer)
    KHEALTHCERTIFICATE_PROPERTY(int, dose, setDose)
    KHEALTHCERTIFICATE_PROPERTY(int, totalDoses, setTotalDoses)
};

Q_DECLARE_METATYPE(KVaccinationCertificate)

#endif // KVACCINATIONCERTIFICATE_H

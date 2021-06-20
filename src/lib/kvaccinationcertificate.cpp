/*
 * SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "kvaccinationcertificate.h"
#include "khealthcertificatetypes_p.h"

class KVaccinationCertificatePrivate : public QSharedData
{
public:
    QString name;
    QDate dateOfBirth;
    QDate date;
    QString vaccine;
    QString manufacturer;
    int dose = 0;
    int totalDoses = 0;
};

KHEALTHCERTIFICATE_MAKE_GADGET(Vaccination)
KHEALTHCERTIFICATE_MAKE_PROPERTY(Vaccination, QString, name, setName)
KHEALTHCERTIFICATE_MAKE_PROPERTY(Vaccination, QDate, dateOfBirth, setDateOfBirth)
KHEALTHCERTIFICATE_MAKE_PROPERTY(Vaccination, QDate, date, setDate)
KHEALTHCERTIFICATE_MAKE_PROPERTY(Vaccination, QString, vaccine, setVaccine)
KHEALTHCERTIFICATE_MAKE_PROPERTY(Vaccination, QString, manufacturer, setManufacturer)
KHEALTHCERTIFICATE_MAKE_PROPERTY(Vaccination, int, dose, setDose)
KHEALTHCERTIFICATE_MAKE_PROPERTY(Vaccination, int, totalDoses, setTotalDoses)

#include "moc_kvaccinationcertificate.cpp"

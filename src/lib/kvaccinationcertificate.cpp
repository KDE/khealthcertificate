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
    QString disease;
    QString vaccine;
    QString manufacturer;
    int dose = 0;
    int totalDoses = 0;
    QString country;
    QString certificateIssuer;
    QString certificateId;
};

KHEALTHCERTIFICATE_MAKE_GADGET(Vaccination)
KHEALTHCERTIFICATE_MAKE_PROPERTY(Vaccination, QString, name, setName)
KHEALTHCERTIFICATE_MAKE_PROPERTY(Vaccination, QDate, dateOfBirth, setDateOfBirth)
KHEALTHCERTIFICATE_MAKE_PROPERTY(Vaccination, QDate, date, setDate)
KHEALTHCERTIFICATE_MAKE_PROPERTY(Vaccination, QString, disease, setDisease)
KHEALTHCERTIFICATE_MAKE_PROPERTY(Vaccination, QString, vaccine, setVaccine)
KHEALTHCERTIFICATE_MAKE_PROPERTY(Vaccination, QString, manufacturer, setManufacturer)
KHEALTHCERTIFICATE_MAKE_PROPERTY(Vaccination, int, dose, setDose)
KHEALTHCERTIFICATE_MAKE_PROPERTY(Vaccination, int, totalDoses, setTotalDoses)
KHEALTHCERTIFICATE_MAKE_PROPERTY(Vaccination, QString, country, setCountry)
KHEALTHCERTIFICATE_MAKE_PROPERTY(Vaccination, QString, certificateIssuer, setCertificateIssuer)
KHEALTHCERTIFICATE_MAKE_PROPERTY(Vaccination, QString, certificateId, setCertificateId)

KHealthCertificate::CertificateValidation KVaccinationCertificate::validationState() const
{
    if (d->date > QDate::currentDate() || (d->dose == 0 && d->totalDoses)) {
        return KHealthCertificate::Invalid;
    }
    if (d->date.addDays(14) >= QDate::currentDate()) {
        return KHealthCertificate::Partial;
    }
    if (d->dose < d->totalDoses) {
        return KHealthCertificate::Partial;
    }

    return KHealthCertificate::Valid;
}

#include "moc_kvaccinationcertificate.cpp"

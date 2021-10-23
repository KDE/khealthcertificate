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
    QString vaccineType;
    QString vaccine;
    QUrl vaccineUrl;
    QString manufacturer;
    int dose = 0;
    int totalDoses = 0;
    QString country;
    QString certificateIssuer;
    QString certificateId;
    QDateTime certificateIssueDate;
    QDateTime certificateExpiryDate;
    QByteArray rawData;
    KHealthCertificate::SignatureValidation signatureState = KHealthCertificate::UnknownSignature;
};

KHEALTHCERTIFICATE_MAKE_GADGET(Vaccination)
KHEALTHCERTIFICATE_MAKE_PROPERTY(Vaccination, QString, name, setName)
KHEALTHCERTIFICATE_MAKE_PROPERTY(Vaccination, QDate, dateOfBirth, setDateOfBirth)
KHEALTHCERTIFICATE_MAKE_PROPERTY(Vaccination, QDate, date, setDate)
KHEALTHCERTIFICATE_MAKE_PROPERTY(Vaccination, QString, disease, setDisease)
KHEALTHCERTIFICATE_MAKE_PROPERTY(Vaccination, QString, vaccineType, setVaccineType)
KHEALTHCERTIFICATE_MAKE_PROPERTY(Vaccination, QString, vaccine, setVaccine)
KHEALTHCERTIFICATE_MAKE_PROPERTY(Vaccination, QUrl, vaccineUrl, setVaccineUrl)
KHEALTHCERTIFICATE_MAKE_PROPERTY(Vaccination, QString, manufacturer, setManufacturer)
KHEALTHCERTIFICATE_MAKE_PROPERTY(Vaccination, int, dose, setDose)
KHEALTHCERTIFICATE_MAKE_PROPERTY(Vaccination, int, totalDoses, setTotalDoses)
KHEALTHCERTIFICATE_MAKE_PROPERTY(Vaccination, QString, country, setCountry)
KHEALTHCERTIFICATE_MAKE_PROPERTY(Vaccination, QString, certificateIssuer, setCertificateIssuer)
KHEALTHCERTIFICATE_MAKE_PROPERTY(Vaccination, QString, certificateId, setCertificateId)
KHEALTHCERTIFICATE_MAKE_PROPERTY(Vaccination, QDateTime, certificateIssueDate, setCertificateIssueDate)
KHEALTHCERTIFICATE_MAKE_PROPERTY(Vaccination, QDateTime, certificateExpiryDate, setCertificateExpiryDate)
KHEALTHCERTIFICATE_MAKE_PROPERTY(Vaccination, QByteArray, rawData, setRawData)
KHEALTHCERTIFICATE_MAKE_PROPERTY(Vaccination, KHealthCertificate::SignatureValidation, signatureState, setSignatureState)

KHealthCertificate::CertificateValidation KVaccinationCertificate::validationState() const
{
    const auto vacState = vaccinationState();
    if (vacState == KVaccinationCertificate::Invalid || d->signatureState == KHealthCertificate::InvalidSignature) {
        return KHealthCertificate::Invalid;
    }

    if ((vacState != KVaccinationCertificate::FullyVaccinated && vacState != KVaccinationCertificate::Vaccinated)
        || d->signatureState == KHealthCertificate::UnknownSignature) {
        return KHealthCertificate::Partial;
    }

    return KHealthCertificate::Valid;
}

KVaccinationCertificate::VaccinationState KVaccinationCertificate::vaccinationState() const
{
    if (d->certificateIssueDate > QDateTime::currentDateTime() || (d->certificateExpiryDate.isValid() && d->certificateExpiryDate < QDateTime::currentDateTime())) {
        return KVaccinationCertificate::Invalid;
    }
    if (d->date > QDate::currentDate() || (d->dose == 0 && d->totalDoses)) {
        return KVaccinationCertificate::Invalid;
    }

    if (d->date.addDays(14) >= QDate::currentDate()) {
        return KVaccinationCertificate::VaccinationTooRecent;
    }
    if (d->dose < d->totalDoses) {
        return KVaccinationCertificate::PartiallyVaccinated;
    }

    return d->totalDoses ? KVaccinationCertificate::FullyVaccinated : KVaccinationCertificate::Vaccinated;
}

#include "moc_kvaccinationcertificate.cpp"

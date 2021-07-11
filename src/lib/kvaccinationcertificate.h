/*
 * SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KVACCINATIONCERTIFICATE_H
#define KVACCINATIONCERTIFICATE_H

#include "khealthcertificate_export.h"
#include "khealthcertificatetypes.h"

#include <QDateTime>
#include <QUrl>

class KVaccinationCertificatePrivate;

/** A vaccination certificate. */
class KHEALTHCERTIFICATE_EXPORT KVaccinationCertificate
{
    KHEALTHCERTIFICATE_GADGET(Vaccination)
    KHEALTHCERTIFICATE_PROPERTY(QString, name, setName)
    KHEALTHCERTIFICATE_PROPERTY(QDate, dateOfBirth, setDateOfBirth)
    KHEALTHCERTIFICATE_PROPERTY(QDate, date, setDate)
    KHEALTHCERTIFICATE_PROPERTY(QString, disease, setDisease)
    /** The vaccine type, such as mRNA or vector. */
    KHEALTHCERTIFICATE_PROPERTY(QString, vaccineType, setVaccineType)
    /** The name of the vaccine as given by the manufacturer. */
    KHEALTHCERTIFICATE_PROPERTY(QString, vaccine, setVaccine)
    /** URL to further details on the vaccine. */
    KHEALTHCERTIFICATE_PROPERTY(QUrl, vaccineUrl, setVaccineUrl)
    /** Name of the manufacturer of the vaccine. */
    KHEALTHCERTIFICATE_PROPERTY(QString, manufacturer, setManufacturer)
    KHEALTHCERTIFICATE_PROPERTY(int, dose, setDose)
    KHEALTHCERTIFICATE_PROPERTY(int, totalDoses, setTotalDoses)
    KHEALTHCERTIFICATE_PROPERTY(QString, country, setCountry)
    /** The entity that issued this certificate. */
    KHEALTHCERTIFICATE_PROPERTY(QString, certificateIssuer, setCertificateIssuer)
    /** The unique identifier of this certificate. */
    KHEALTHCERTIFICATE_PROPERTY(QString, certificateId, setCertificateId)
    /** Date/time this certificate has been issued at. */
    KHEALTHCERTIFICATE_PROPERTY(QDateTime, certificateIssueDate, setCertificateIssueDate)
    /** Date/time this certificate expires. */
    KHEALTHCERTIFICATE_PROPERTY(QDateTime, certificateExpiryDate, setCertificateExpiryDate)
    /** Validation status of the cryptographic signature of this certificate. */
    KHEALTHCERTIFICATE_PROPERTY(KHealthCertificate::SignatureValidation, signatureState, setSignatureState)

    Q_PROPERTY(KHealthCertificate::CertificateValidation validationState READ validationState)

    /** Fully encoded data as represented in the barcode. */
    KHEALTHCERTIFICATE_PROPERTY(QByteArray, rawData, setRawData)
public:
    KHealthCertificate::CertificateValidation validationState() const;
};

Q_DECLARE_METATYPE(KVaccinationCertificate)

#endif // KVACCINATIONCERTIFICATE_H

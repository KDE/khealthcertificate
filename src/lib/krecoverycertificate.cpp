/*
 * SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "krecoverycertificate.h"
#include "khealthcertificatetypes_p.h"

class KRecoveryCertificatePrivate : public QSharedData
{
public:
    QString name;
    QDate dateOfBirth;
    QDate dateOfPositiveTest;
    QDate validFrom;
    QDate validUntil;
    QString disease;
    QString certificateIssuer;
    QString certificateId;
    QDateTime certificateIssueDate;
    QDateTime certificateExpiryDate;
    QByteArray rawData;
    KHealthCertificate::SignatureValidation signatureState = KHealthCertificate::UnknownSignature;
};

KHEALTHCERTIFICATE_MAKE_GADGET(Recovery)
KHEALTHCERTIFICATE_MAKE_PROPERTY(Recovery, QString, name, setName)
KHEALTHCERTIFICATE_MAKE_PROPERTY(Recovery, QDate, dateOfBirth, setDateOfBirth)
KHEALTHCERTIFICATE_MAKE_PROPERTY(Recovery, QDate, dateOfPositiveTest, setDateOfPositiveTest)
KHEALTHCERTIFICATE_MAKE_PROPERTY(Recovery, QDate, validFrom, setValidFrom)
KHEALTHCERTIFICATE_MAKE_PROPERTY(Recovery, QDate, validUntil, setValidUntil)
KHEALTHCERTIFICATE_MAKE_PROPERTY(Recovery, QString, disease, setDisease)
KHEALTHCERTIFICATE_MAKE_PROPERTY(Recovery, QString, certificateId, setCertificateId)
KHEALTHCERTIFICATE_MAKE_PROPERTY(Recovery, QString, certificateIssuer, setCertificateIssuer)
KHEALTHCERTIFICATE_MAKE_PROPERTY(Recovery, QDateTime, certificateIssueDate, setCertificateIssueDate)
KHEALTHCERTIFICATE_MAKE_PROPERTY(Recovery, QDateTime, certificateExpiryDate, setCertificateExpiryDate)
KHEALTHCERTIFICATE_MAKE_PROPERTY(Recovery, QByteArray, rawData, setRawData)
KHEALTHCERTIFICATE_MAKE_PROPERTY(Recovery, KHealthCertificate::SignatureValidation, signatureState, setSignatureState)

KHealthCertificate::CertificateValidation KRecoveryCertificate::validationState() const
{
    if (d->certificateIssueDate > QDateTime::currentDateTime() || (d->certificateExpiryDate.isValid() && d->certificateExpiryDate < QDateTime::currentDateTime())) {
        return KHealthCertificate::Invalid;
    }
    if (d->signatureState == KHealthCertificate::InvalidSignature) {
        return KHealthCertificate::Invalid;
    }

    if (d->signatureState == KHealthCertificate::UnknownSignature) {
        return KHealthCertificate::Partial;
    }

    const auto today = QDate::currentDate();
    if (d->validFrom.isValid() && d->validFrom > today) {
        return KHealthCertificate::Invalid;
    }
    if (d->validUntil.isValid() && d->validUntil < today) {
        return KHealthCertificate::Invalid;
    }

    if (d->validFrom.isValid() && d->validFrom <= today && d->validUntil.isValid() && d->validUntil >= today) {
        return KHealthCertificate::Valid;
    }

    return KHealthCertificate::Unknown;
}

#include "moc_krecoverycertificate.cpp"

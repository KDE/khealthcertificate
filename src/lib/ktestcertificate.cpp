/*
 * SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "ktestcertificate.h"
#include "khealthcertificatetypes_p.h"

class KTestCertificatePrivate : public QSharedData
{
public:
    QString name;
    QDate dateOfBirth;
    QDate date;
    QString disease;
    QString testType;
    QString naaTestName;
    QString ratTest;
    KTestCertificate::Result result = KTestCertificate::Unknown;
    QString resultString;
    QString testCenter;
    QString country;
    QString certificateIssuer;
    QString certificateId;
    QDateTime certificateIssueDate;
    QDateTime certificateExpiryDate;
    QByteArray rawData;
    KHealthCertificate::SignatureValidation signatureState = KHealthCertificate::UnknownSignature;
};

KHEALTHCERTIFICATE_MAKE_GADGET(Test)
KHEALTHCERTIFICATE_MAKE_PROPERTY(Test, QString, name, setName)
KHEALTHCERTIFICATE_MAKE_PROPERTY(Test, QDate, dateOfBirth, setDateOfBirth)
KHEALTHCERTIFICATE_MAKE_PROPERTY(Test, QDate, date, setDate)
KHEALTHCERTIFICATE_MAKE_PROPERTY(Test, QString, disease, setDisease)
KHEALTHCERTIFICATE_MAKE_PROPERTY(Test, QString, testType, setTestType)
KHEALTHCERTIFICATE_MAKE_PROPERTY(Test, QString, naaTestName, setNaaTestName)
KHEALTHCERTIFICATE_MAKE_PROPERTY(Test, QString, ratTest, setRatTest)
KHEALTHCERTIFICATE_MAKE_PROPERTY(Test, KTestCertificate::Result, result, setResult)
KHEALTHCERTIFICATE_MAKE_PROPERTY(Test, QString, resultString, setResultString)
KHEALTHCERTIFICATE_MAKE_PROPERTY(Test, QString, testCenter, setTestCenter)
KHEALTHCERTIFICATE_MAKE_PROPERTY(Test, QString, country, setCountry)
KHEALTHCERTIFICATE_MAKE_PROPERTY(Test, QString, certificateIssuer, setCertificateIssuer)
KHEALTHCERTIFICATE_MAKE_PROPERTY(Test, QString, certificateId, setCertificateId)
KHEALTHCERTIFICATE_MAKE_PROPERTY(Test, QDateTime, certificateIssueDate, setCertificateIssueDate)
KHEALTHCERTIFICATE_MAKE_PROPERTY(Test, QDateTime, certificateExpiryDate, setCertificateExpiryDate)
KHEALTHCERTIFICATE_MAKE_PROPERTY(Test, QByteArray, rawData, setRawData)
KHEALTHCERTIFICATE_MAKE_PROPERTY(Test, KHealthCertificate::SignatureValidation, signatureState, setSignatureState)

KHealthCertificate::CertificateValidation KTestCertificate::validationState() const
{
    if (d->certificateIssueDate > QDateTime::currentDateTime() || (d->certificateExpiryDate.isValid() && d->certificateExpiryDate < QDateTime::currentDateTime())) {
        return KHealthCertificate::Invalid;
    }
    if (d->signatureState == KHealthCertificate::InvalidSignature) {
        return KHealthCertificate::Invalid;
    }
    if (d->result == Positive) {
        return KHealthCertificate::Invalid;
    }

    if (d->date.addDays(2) < QDate::currentDate()) {
        return KHealthCertificate::Invalid;
    }
    if (d->signatureState == KHealthCertificate::UnknownSignature) {
        return KHealthCertificate::Partial;
    }

    return d->result == Negative ? KHealthCertificate::Valid : KHealthCertificate::Unknown;
}

#include "moc_ktestcertificate.moc"

/*
 * SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KTESTCERTIFICATE_H
#define KTESTCERTIFICATE_H

#include "khealthcertificate_export.h"
#include "khealthcertificatetypes.h"

#include <QDate>

class KTestCertificatePrivate;

/** A test certificate. */
class KHEALTHCERTIFICATE_EXPORT KTestCertificate
{
    KHEALTHCERTIFICATE_GADGET(Test)
    KHEALTHCERTIFICATE_PROPERTY(QString, name, setName)
    KHEALTHCERTIFICATE_PROPERTY(QDate, dateOfBirth, setDateOfBirth)
    KHEALTHCERTIFICATE_PROPERTY(QDate, date, setDate)
    KHEALTHCERTIFICATE_PROPERTY(QString, disease, setDisease)
    KHEALTHCERTIFICATE_PROPERTY(QString, testType, setTestType)
    KHEALTHCERTIFICATE_PROPERTY(QString, naaTestName, setNaaTestName)
    KHEALTHCERTIFICATE_PROPERTY(QString, ratTest, setRatTest)
    KHEALTHCERTIFICATE_PROPERTY(QString, resultString, setResultString)
    KHEALTHCERTIFICATE_PROPERTY(QString, testCenter, setTestCenter)
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

    enum Result {
        Unknown,
        Negative,
        Positive,
    };
    Q_ENUM(Result)
    KHEALTHCERTIFICATE_PROPERTY(Result, result, setResult)

    /** Fully encoded data as represented in the barcode. */
    KHEALTHCERTIFICATE_PROPERTY(QByteArray, rawData, setRawData)
public:
    KHealthCertificate::CertificateValidation validationState() const;};

Q_DECLARE_METATYPE(KTestCertificate)

#endif // KTESTCERTIFICATE_H

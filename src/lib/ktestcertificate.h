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
    KHEALTHCERTIFICATE_PROPERTY(QString, testCenter, setTextCenter)
    KHEALTHCERTIFICATE_PROPERTY(QString, country, setCountry)
    Q_PROPERTY(KHealthCertificate::CertificateValidation validationState READ validationState)

    enum Result {
        Unknown,
        Negative,
        Positive,
    };
    Q_ENUM(Result)
    KHEALTHCERTIFICATE_PROPERTY(Result, result, setResult)

public:
    KHealthCertificate::CertificateValidation validationState() const;};

Q_DECLARE_METATYPE(KTestCertificate)

#endif // KTESTCERTIFICATE_H

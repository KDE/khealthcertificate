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
    QString result;
    QString testCenter;
    QString country;
};

KHEALTHCERTIFICATE_MAKE_GADGET(Test)
KHEALTHCERTIFICATE_MAKE_PROPERTY(Test, QString, name, setName)
KHEALTHCERTIFICATE_MAKE_PROPERTY(Test, QDate, dateOfBirth, setDateOfBirth)
KHEALTHCERTIFICATE_MAKE_PROPERTY(Test, QDate, date, setDate)
KHEALTHCERTIFICATE_MAKE_PROPERTY(Test, QString, disease, setDisease)
KHEALTHCERTIFICATE_MAKE_PROPERTY(Test, QString, testType, setTestType)
KHEALTHCERTIFICATE_MAKE_PROPERTY(Test, QString, naaTestName, setNaaTestName)
KHEALTHCERTIFICATE_MAKE_PROPERTY(Test, QString, ratTest, setRatTest)
KHEALTHCERTIFICATE_MAKE_PROPERTY(Test, QString, result, setResult)
KHEALTHCERTIFICATE_MAKE_PROPERTY(Test, QString, testCenter, setTextCenter)
KHEALTHCERTIFICATE_MAKE_PROPERTY(Test, QString, country, setCountry)

#include "moc_ktestcertificate.moc"

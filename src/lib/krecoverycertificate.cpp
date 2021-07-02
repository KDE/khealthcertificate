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

KHealthCertificate::CertificateValidation KRecoveryCertificate::validationState() const
{
    return KHealthCertificate::Unknown;
}

#include "moc_krecoverycertificate.cpp"

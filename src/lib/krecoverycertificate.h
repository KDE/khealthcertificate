/*
 * SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KRECOVERYCERTIFICATE_H
#define KRECOVERYCERTIFICATE_H

#include "khealthcertificate_export.h"
#include "khealthcertificatetypes.h"

#include <QDate>

class KRecoveryCertificatePrivate;

/** A recovery certificate. */
class KHEALTHCERTIFICATE_EXPORT KRecoveryCertificate
{
    KHEALTHCERTIFICATE_GADGET(Recovery)
    KHEALTHCERTIFICATE_PROPERTY(QString, name, setName)
    KHEALTHCERTIFICATE_PROPERTY(QDate, dateOfBirth, setDateOfBirth)
    KHEALTHCERTIFICATE_PROPERTY(QDate, dateOfPositiveTest, setDateOfPositiveTest)
    KHEALTHCERTIFICATE_PROPERTY(QDate, validFrom, setValidFrom)
    KHEALTHCERTIFICATE_PROPERTY(QDate, validUntil, setValidUntil)
    KHEALTHCERTIFICATE_PROPERTY(QString, disease, setDisease)
    /** The entity that issued this certificate. */
    KHEALTHCERTIFICATE_PROPERTY(QString, certificateIssuer, setCertificateIssuer)
    /** The unique identifier of this certificate. */
    KHEALTHCERTIFICATE_PROPERTY(QString, certificateId, setCertificateId)
    /** Date/time this certificate has been issued at. */
    KHEALTHCERTIFICATE_PROPERTY(QDateTime, certificateIssueDate, setCertificateIssueDate)
    /** Date/time this certificate expires. */
    KHEALTHCERTIFICATE_PROPERTY(QDateTime, certificateExpiryDate, setCertificateExpiryDate)

    Q_PROPERTY(KHealthCertificate::CertificateValidation validationState READ validationState)

public:
    KHealthCertificate::CertificateValidation validationState() const;
};

Q_DECLARE_METATYPE(KRecoveryCertificate)

#endif // KRECOVERYCERTIFICATE_H

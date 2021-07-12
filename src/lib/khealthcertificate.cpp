/*
 * SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "khealthcertificate.h"
#include "krecoverycertificate.h"
#include "ktestcertificate.h"
#include "kvaccinationcertificate.h"

#include <QDateTime>
#include <QVariant>

QDateTime KHealthCertificate::relevantUntil(const QVariant &certificate)
{
    if (certificate.userType() == qMetaTypeId<KVaccinationCertificate>()) {
        const auto vac = certificate.value<KVaccinationCertificate>();
        if (vac.certificateExpiryDate().isValid()) {
            return vac.certificateExpiryDate();
        }
        return QDateTime(vac.date().addYears(1), {0, 0});
    }

    if (certificate.userType() == qMetaTypeId<KTestCertificate>()) {
        const auto test = certificate.value<KTestCertificate>();
        return QDateTime(test.date().addDays(2), {0, 0});
    }

    if (certificate.userType() == qMetaTypeId<KRecoveryCertificate>()) {
        const auto rec = certificate.value<KRecoveryCertificate>();
        return QDateTime(rec.validUntil(), {0, 0});
    }

    return {};
}

#include "moc_khealthcertificate.cpp"

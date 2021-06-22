/*
 * SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KHEALTHCERTIFICATE_H
#define KHEALTHCERTIFICATE_H

#include "khealthcertificate_export.h"
#include <QMetaType>

/** Dummy RTTI for QML, which doesn't support `instanceof` on Q_GADGETs... */
class KHEALTHCERTIFICATE_EXPORT KHealthCertificate
{
    Q_GADGET
public:
    enum CertificateType {
        Vaccination,
        Test,
        Recovery,
    };
    Q_ENUM(CertificateType)

    enum CertificateValidation {
        Valid,
        Partial,
        Invalid,
        Unknown
    };
    Q_ENUM(CertificateValidation)
};

#endif // KHEALTHCERTIFICATE_H

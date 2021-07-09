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

    /** Result of attempting to verify the cryptographic signature of a certificate. */
    enum SignatureValidation {
        ValidSignature, ///< signature is valid
        InvalidSignature, ///< signature is invalid
        UnknownSignature, ///< signature verification was attempted but didn't yield a result, e.g. due to a missing certificate of signing entity.
        UncheckedSignature, ///< signature verification was not attempted, e.g. as it's not yet implemented for the specific certificate type.
    };
    Q_ENUM(SignatureValidation)
};

#endif // KHEALTHCERTIFICATE_H

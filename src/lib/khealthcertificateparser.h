/*
 * SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KHEALTHCERTIFICATEPARSER_H
#define KHEALTHCERTIFICATEPARSER_H

#include "khealthcertificate_export.h"

class QByteArray;
class QVariant;

/** Parses health certificates. */
namespace KHealthCertificateParser
{
    /**
     * Parse a single digital health certificate.
     *
     * @param data The digital health certificate, typically this is the content of a QR code.
     * Input can be of any supported format, the exact type is auto-detected.
     *
     * @returns Four different things can be returned, depending on the input.
     * - KVaccinationCertificate
     * - KTestCertificate
     * - KRecoveryCertificate
     * - a null QVariant in case of invalid or unsupported input
     * If the input was parsed successfully but is semantically invalid that needs to be checked
     * separately, see e.g. KVaccinationCertificate::signatureState and KVaccinationCertificate::validationState.
     */
    KHEALTHCERTIFICATE_EXPORT QVariant parse(const QByteArray &data);
}

#endif // KHEALTHCERTIFICATEPARSER_H

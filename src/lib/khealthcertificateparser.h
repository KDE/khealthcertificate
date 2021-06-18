/*
 * SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KHEALTHCERTIFICATEPARSER_H
#define KHEALTHCERTIFICATEPARSER_H

#include "khealthcertificate_export.h"

class QByteArray;
class QVariant;

/** Parses raw health certificate data received from a barcode scanner. */
namespace KHealthCertificateParser
{
    KHEALTHCERTIFICATE_EXPORT QVariant parse(const QByteArray &data);
}

#endif // KHEALTHCERTIFICATEPARSER_H

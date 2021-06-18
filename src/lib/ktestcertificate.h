/*
 * SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KTESTCERTIFICATE_H
#define KTESTCERTIFICATE_H

#include "khealthcertificate_export.h"
#include "khealthcertificatetypes.h"

class KTestCertificatePrivate;

/** A test certificate. */
class KHEALTHCERTIFICATE_EXPORT KTestCertificate
{
    KHEALTHCERTIFICATE_GADGET(Test)
    KHEALTHCERTIFICATE_PROPERTY(QString, name, setName)
};

Q_DECLARE_METATYPE(KTestCertificate)

#endif // KTESTCERTIFICATE_H

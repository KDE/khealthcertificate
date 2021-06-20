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
};

Q_DECLARE_METATYPE(KRecoveryCertificate)

#endif // KRECOVERYCERTIFICATE_H

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
};

KHEALTHCERTIFICATE_MAKE_GADGET(Recovery)
KHEALTHCERTIFICATE_MAKE_PROPERTY(Recovery, QString, name, setName)

#include "moc_krecoverycertificate.cpp"

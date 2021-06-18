/*
 * SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "ktestcertificate.h"
#include "khealthcertificatetypes_p.h"

class KTestCertificatePrivate : public QSharedData
{
public:
    QString name;
};

KHEALTHCERTIFICATE_MAKE_GADGET(Test)
KHEALTHCERTIFICATE_MAKE_PROPERTY(Test, QString, name, setName)

#include "moc_ktestcertificate.moc"

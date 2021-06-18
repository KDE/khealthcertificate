/*
 * SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include <KHealthCertificate/KHealthCertificateParser>
#include <KHealthCertificate/KRecoveryCertificate>
#include <KHealthCertificate/KTestCertificate>
#include <KHealthCertificate/KVaccinationCertificate>

#include <QDebug>
#include <QFile>
#include <QMetaProperty>
#include <QVariant>

#include <iostream>

static void dumpGadget(const QVariant &gadget)
{
    const auto mo = QMetaType(gadget.userType()).metaObject();
    if (!mo) {
        return;
    }

    for (auto i = 0; i < mo->propertyCount(); ++i) {
        const auto prop = mo->property(i);
        if (!prop.isStored()) {
            continue;
        }
        const auto value = prop.readOnGadget(gadget.constData());
        std::cout << " " << prop.name() << ": " << qPrintable(value.toString()) << std::endl;
    }
}

int main(int argc, char **argv)
{
    // TODO cli arg parsing

    QFile in;
    in.open(stdin, QFile::ReadOnly);

    const auto cert = KHealthCertificateParser::parse(in.readAll());
    if (cert.isNull()) {
        std::cout << "unable to parse input" << std::endl;
        return 1;
    }

    if (cert.userType() == qMetaTypeId<KVaccinationCertificate>()) {
        std::cout << "Vaccination certificate:" << std::endl;
    } else if (cert.userType() == qMetaTypeId<KTestCertificate>()) {
        std::cout << "Test certificate:" << std::endl;
    } else if (cert.userType() == qMetaTypeId<KRecoveryCertificate>()) {
        std::cout << "Recovery certificate:" << std::endl;
    }
    dumpGadget(cert);

    return 0;
}

/*
 * SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "irmapublickey_p.h"

#include "openssl/bignum_p.h"

#include <QDebug>
#include <QFile>
#include <QXmlStreamReader>

// see https://pkg.go.dev/github.com/privacybydesign/gabi@v0.0.0-20210816093228-75a6590e506c/gabikeys#PublicKey

IrmaPublicKey::IrmaPublicKey()
    : N(openssl::bn_ptr(nullptr, &BN_free))
    , Z(openssl::bn_ptr(nullptr, &BN_free))
    , S(openssl::bn_ptr(nullptr, &BN_free))
{
}

bool IrmaPublicKey::isValid() const
{
    return N && Z && S && std::all_of(R.begin(), R.end(), [](const auto &r) { return r.get() != nullptr; });
}


IrmaPublicKey IrmaPublicKeyLoader::load(const QString &keyId)
{
    IrmaPublicKey pk;

    QFile pkFile(QLatin1String(":/org.kde.khealthcertificate/nl-coronacheck/keys/") + keyId + QLatin1String(".xml"));
    if (!pkFile.open(QFile::ReadOnly)) {
        qWarning() << "Failed to find IRMA public key:" << keyId;
        return pk;
    }

    QXmlStreamReader reader(&pkFile);
    while (!reader.atEnd() && !reader.hasError()) {
        reader.readNextStartElement();
        if (reader.name() == QLatin1String("n")) {
            pk.N = Bignum::fromDecimalString(reader.readElementText());
        }
        else if (reader.name() == QLatin1String("Z")) {
            pk.Z = Bignum::fromDecimalString(reader.readElementText());
        }
        else if (reader.name() == QLatin1String("S")) {
            pk.S = Bignum::fromDecimalString(reader.readElementText());
        }
        else if (reader.name() == QLatin1String("Bases")) {
            const auto num = reader.attributes().value(QLatin1String("num")).toInt();
            pk.R.reserve(num);
        }
        else if (reader.name().startsWith(QLatin1String("Base_"))) {
            pk.R.push_back(Bignum::fromDecimalString(reader.readElementText()));
        }
    }

    return pk;
}

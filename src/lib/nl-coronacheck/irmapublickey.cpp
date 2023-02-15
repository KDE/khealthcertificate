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

IrmaPublicKey::IrmaPublicKey() = default;

bool IrmaPublicKey::isValid() const
{
    return N && Z && S && std::all_of(R.begin(), R.end(), [](const auto &r) { return r.get() != nullptr; });
}

// see https://github.com/minvws/gabi/blob/idemix_origin/sysparams.go#L21
int IrmaPublicKey::LePrime() const
{
    return 120;
}

int IrmaPublicKey::Lh() const
{
    return 256;
}

int IrmaPublicKey::Lm() const
{
    switch (BN_num_bits(N.get())) {
        case 1024:
        case 2048:
            return 256;
        case 4096:
            return 512;
    }
    return 0;
}

int IrmaPublicKey::Lstatzk() const
{
    switch (BN_num_bits(N.get())) {
        case 1024:
            return 80;
        case 2048:
        case 4096:
            return 128;
    }
    return 0;
}

// and https://github.com/minvws/gabi/blob/idemix_origin/sysparams.go#L60
int IrmaPublicKey::Le() const
{
    return Lstatzk() + Lh() + Lm() + 5;
}

int IrmaPublicKey::LeCommit() const
{
    return LePrime() + Lstatzk() + Lh();
}

int IrmaPublicKey::LmCommit() const
{
    return Lm() + Lstatzk() + Lh();
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

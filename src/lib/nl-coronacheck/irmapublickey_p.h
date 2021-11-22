/*
 * SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef IRMAPUBLICKEY_H
#define IRMAPUBLICKEY_H

#include "openssl/opensslpp_p.h"

#include <vector>

class QString;

/** Public key data for the Dutch IRMA system
 *  @note this only covers the subset relevant for verifying CoronaCheck signatures
 *.*/
class IrmaPublicKey
{
public:
    explicit IrmaPublicKey();

    openssl::bn_ptr N;
    openssl::bn_ptr Z;
    openssl::bn_ptr S;
    std::vector <openssl::bn_ptr> R;
};

/** Loader for IRMA public keys. */
namespace IrmaPublicKeyLoader
{
    IrmaPublicKey load(const QString &keyId);
}

#endif // IRMAPUBLICKEY_H

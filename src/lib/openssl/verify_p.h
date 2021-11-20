/*
 * SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef VERIFY_P_H
#define VERIFY_P_H

#include "opensslpp_p.h"

/** Signature verification utilities. */
namespace Verify
{
    bool verifyECDSA(
        const openssl::evp_pkey_ptr &pkey, const EVP_MD *digest,
        const char *data, std::size_t dataSize,
        const char *signature, std::size_t signatureSize);
}

#endif // VERIFY_P_H

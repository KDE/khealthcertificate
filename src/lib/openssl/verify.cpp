/*
 * SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "verify_p.h"
#include "logging.h"

#include "openssl/bignum_p.h"

#include <openssl/err.h>

bool Verify::verifyECDSA(
    const openssl::evp_pkey_ptr &pkey, const EVP_MD *digest,
    const char *data, std::size_t dataSize,
    const char *signature, std::size_t signatureSize)
{
    if (!pkey) {
        qCWarning(Log) << "no key provided";
        return false;
    }

    const openssl::ec_key_ptr ecKey(EVP_PKEY_get1_EC_KEY(pkey.get()), &EC_KEY_free);

    // compute hash of the signed data
    uint8_t digestData[EVP_MAX_MD_SIZE];
    uint32_t  digestSize = 0;
    EVP_Digest(reinterpret_cast<const uint8_t*>(data), dataSize, digestData, &digestSize, digest, nullptr);
    if (digestSize * 2 != signatureSize || EVP_PKEY_bits(pkey.get()) != 4 * (int)signatureSize) {
        qCWarning(Log) << "digest size mismatch!?" << digestSize << signatureSize;
        return false;
    }

    // unpack the signature field
    auto r = Bignum::fromByteArray(signature, signatureSize / 2);
    auto s = Bignum::fromByteArray(signature + signatureSize / 2, signatureSize / 2);

    // verify
    const openssl::ecdsa_sig_ptr sig(ECDSA_SIG_new(), &ECDSA_SIG_free);
    ECDSA_SIG_set0(sig.get(), r.release(), s.release());
    const auto verifyResult = ECDSA_do_verify(digestData, digestSize, sig.get(), ecKey.get());
    switch (verifyResult) {
        case -1: // technical issue
            qCWarning(Log) << "Failed to verify signature:" << ERR_error_string(ERR_get_error(), nullptr);
            return false;
        case 0: // invalid signature
            return false;
        case 1: // valid signature;
            return true;
    }

    Q_UNREACHABLE();
    return false;
}

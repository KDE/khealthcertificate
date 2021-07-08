/*
 * SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KHEALTHCERTIFICATE_OPENSSLPP_H
#define KHEALTHCERTIFICATE_OPENSSLPP_H

#include <memory>

#include <openssl/bio.h>
#include <openssl/ecdh.h>
#include <openssl/ecdsa.h>
#include <openssl/evp.h>
#include <openssl/x509.h>

namespace openssl {
    using bio_ptr = std::unique_ptr<BIO, decltype(&BIO_free_all)>;
    using ec_key_ptr = std::unique_ptr<EC_KEY, decltype(&EC_KEY_free)>;
    using ecdsa_sig_ptr = std::unique_ptr<ECDSA_SIG, decltype(&ECDSA_SIG_free)>;
    using evp_pkey_ptr = std::unique_ptr<EVP_PKEY, decltype(&EVP_PKEY_free)>;
    using x509_ptr = std::unique_ptr<X509, decltype(&X509_free)>;
}

#endif

/*
 * SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KHEALTHCERTIFICATE_OPENSSLPP_P_H
#define KHEALTHCERTIFICATE_OPENSSLPP_P_H

#include <memory>

#include <openssl/asn1.h>
#include <openssl/bio.h>
#include <openssl/bn.h>
#include <openssl/ecdh.h>
#include <openssl/ecdsa.h>
#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/x509.h>

namespace openssl {
    using asn1_integer_ptr = std::unique_ptr<ASN1_INTEGER, decltype(&ASN1_INTEGER_free)>;
    using asn1_octet_string_ptr = std::unique_ptr<ASN1_OCTET_STRING, decltype(&ASN1_OCTET_STRING_free)>;
    using asn1_printable_string_ptr = std::unique_ptr<ASN1_PRINTABLESTRING, decltype(&ASN1_PRINTABLESTRING_free)>;
    using bio_ptr = std::unique_ptr<BIO, decltype(&BIO_free_all)>;
    using bn_ptr = std::unique_ptr<BIGNUM, decltype(&BN_free)>;
    using bn_ctx_ptr = std::unique_ptr<BN_CTX, decltype(&BN_CTX_free)>;
    using ec_key_ptr = std::unique_ptr<EC_KEY, decltype(&EC_KEY_free)>;
    using ecdsa_sig_ptr = std::unique_ptr<ECDSA_SIG, decltype(&ECDSA_SIG_free)>;
    using evp_pkey_ptr = std::unique_ptr<EVP_PKEY, decltype(&EVP_PKEY_free)>;
    using evp_pkey_ctx_ptr = std::unique_ptr<EVP_PKEY_CTX, decltype(&EVP_PKEY_CTX_free)>;
    using rsa_ptr = std::unique_ptr<RSA, decltype(&RSA_free)>;
    using x509_ptr = std::unique_ptr<X509, decltype(&X509_free)>;
}

#endif

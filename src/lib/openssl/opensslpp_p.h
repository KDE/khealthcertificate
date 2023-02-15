/*
 * SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KHEALTHCERTIFICATE_OPENSSLPP_P_H
#define KHEALTHCERTIFICATE_OPENSSLPP_P_H

#include <functional>
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

    namespace detail {
        template <typename T, void(*F)(T*)>
        struct deleter {
            void operator()(T *ptr) { std::invoke(F, ptr); }
        };
    }

    using asn1_integer_ptr = std::unique_ptr<ASN1_INTEGER, detail::deleter<ASN1_INTEGER, &ASN1_INTEGER_free>>;
    using asn1_octet_string_ptr = std::unique_ptr<ASN1_OCTET_STRING, detail::deleter<ASN1_OCTET_STRING, &ASN1_OCTET_STRING_free>>;
    using asn1_printable_string_ptr = std::unique_ptr<ASN1_PRINTABLESTRING, detail::deleter<ASN1_PRINTABLESTRING, &ASN1_PRINTABLESTRING_free>>;
    using asn1_type_ptr = std::unique_ptr<ASN1_TYPE, detail::deleter<ASN1_TYPE, &ASN1_TYPE_free>>;
    using bio_ptr = std::unique_ptr<BIO, detail::deleter<BIO, &BIO_free_all>>;
    using bn_ptr = std::unique_ptr<BIGNUM, detail::deleter<BIGNUM, &BN_free>>;
    using bn_ctx_ptr = std::unique_ptr<BN_CTX, detail::deleter<BN_CTX, &BN_CTX_free>>;
    using ec_key_ptr = std::unique_ptr<EC_KEY, detail::deleter<EC_KEY, &EC_KEY_free>>;
    using ecdsa_sig_ptr = std::unique_ptr<ECDSA_SIG, detail::deleter<ECDSA_SIG, &ECDSA_SIG_free>>;
    using evp_pkey_ptr = std::unique_ptr<EVP_PKEY, detail::deleter<EVP_PKEY, &EVP_PKEY_free>>;
    using evp_pkey_ctx_ptr = std::unique_ptr<EVP_PKEY_CTX, detail::deleter<EVP_PKEY_CTX, &EVP_PKEY_CTX_free>>;
    using rsa_ptr = std::unique_ptr<RSA, detail::deleter<RSA, &RSA_free>>;
    using x509_ptr = std::unique_ptr<X509, detail::deleter<X509, &X509_free>>;
}

#endif

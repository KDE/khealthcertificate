/*
 * SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KHEALTHCERTIFICATE_BIGNUM_P_H
#define KHEALTHCERTIFICATE_BIGNUM_P_H

#include "opensslpp_p.h"

#include <QByteArray>
#include <QString>

/** Utilities for working with OpenSSL BIGNUM objects. */
class Bignum
{
public:
    /** @see BN_bin2bn */
    static inline openssl::bn_ptr fromByteArray(const char *bin, std::size_t size)
    {
        return openssl::bn_ptr(BN_bin2bn(reinterpret_cast<const uint8_t*>(bin), size, nullptr));
    }
    static inline openssl::bn_ptr fromByteArray(const QByteArray &bin)
    {
        return fromByteArray(bin.constData(), bin.size());
    }

    /** @see BN_bn2bin */
    static inline QByteArray toByteArray(const openssl::bn_ptr &bn)
    {
        QByteArray bin;
        bin.resize(BN_num_bytes(bn.get()));
        BN_bn2bin(bn.get(), reinterpret_cast<uint8_t*>(bin.data()));
        return bin;
    }

    /** @see BN_dec2bn */
    static inline openssl::bn_ptr fromDecimalString(const QByteArray &dec)
    {
        BIGNUM *bn = nullptr;
        BN_dec2bn(&bn, dec.constData());
        return openssl::bn_ptr(bn);
    }
    static inline openssl::bn_ptr fromDecimalString(const QString &dec)
    {
        return fromDecimalString(dec.toLatin1());
    }
};

#endif

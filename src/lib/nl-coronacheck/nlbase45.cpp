/*
 * SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "nlbase45_p.h"

#include "openssl/opensslpp_p.h"

#include <QByteArray>
#include <QDebug>

#include <algorithm>

static constexpr const char nlBase45Table[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ $%*+-./:";

static int8_t nlBase45MapFromChar(char c)
{
    const auto it = std::find(std::begin(nlBase45Table), std::end(nlBase45Table), c);
    if (it == std::end(nlBase45Table)) {
        qWarning() << "invalid base45 character:" << c;
        return -1;
    }
    return std::distance(std::begin(nlBase45Table), it);
}

QByteArray NLBase45::decode(const char *begin, const char *end)
{
    openssl::bn_ptr bn(BN_new(), &BN_free);
    BN_zero(bn.get());
    for (auto it = begin; it != end; ++it) {
        BN_mul_word(bn.get(), 45);
        auto v = nlBase45MapFromChar(*it);
        if (v < 0) {
            break;
        }
        BN_add_word(bn.get(), v);
    }

    QByteArray out;
    out.resize(BN_num_bytes(bn.get()));
    BN_bn2bin(bn.get(), reinterpret_cast<uint8_t*>(out.data()));

    return out;
}

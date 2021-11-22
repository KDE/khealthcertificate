/*
 * SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef IRMAVERIFIER_H
#define IRMAVERIFIER_H

#include "openssl/opensslpp_p.h"

#include <vector>

class IrmaPublicKey;

class QByteArray;

class IrmaProof
{
public:
    explicit IrmaProof();

    int64_t disclosureTime;
    openssl::bn_ptr C;
    openssl::bn_ptr A;
    openssl::bn_ptr EResponse;
    openssl::bn_ptr VResponse;
    std::vector <openssl::bn_ptr> AResponses;
    std::vector <openssl::bn_ptr> ADisclosed;
};

#endif // IRMAVERIFIER_H

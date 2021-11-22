/*
 * SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "irmaverifier_p.h"
#include "irmapublickey_p.h"

#include <QCryptographicHash>
#include <QDebug>

IrmaProof::IrmaProof()
    : disclosureTime(0)
    , C(openssl::bn_ptr(nullptr, &BN_free))
    , A(openssl::bn_ptr(nullptr, &BN_free))
    , EResponse(openssl::bn_ptr(nullptr, &BN_free))
    , VResponse(openssl::bn_ptr(nullptr, &BN_free))
{
}

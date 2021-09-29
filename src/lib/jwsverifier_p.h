/*
 * SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef JWSVERIFIER_H
#define JWSVERIFIER_H

#include "opensslpp_p.h"

#include <QJsonObject>

/** Verification of JSON Web Signatures (JWS).
 *  @see RFC 7515
 *  @see RFC 7797 (unencoded payload extension)
 *
 *  @note This is far from a complete implementation of the full spec, this barely
 *  covers enough for the needs of DIVOC JWS verification.
 */
class JwsVerifier
{
public:
    explicit JwsVerifier(const QJsonObject &doc);
    ~JwsVerifier();

    bool verify() const;

private:
    openssl::evp_pkey_ptr loadPublicKey() const;
    QByteArray canonicalRdf(const QJsonObject &doc) const;

    QJsonObject m_obj;
};

#endif // JWSVERIFIER_H

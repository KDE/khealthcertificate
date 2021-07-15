/*
 * SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef JWSVERIFIER_H
#define JWSVERIFIER_H

#include <QJsonObject>

/** Verification of JSON Web Signatures (JWS).
 *  @see RFC 7515
 *  @see RFC 7797 (unencoded payload extension)
 */
class JwsVerifier
{
public:
    JwsVerifier(const QJsonObject &doc);
    ~JwsVerifier();

    bool verify() const;

private:
    QJsonObject m_obj;
};

#endif // JWSVERIFIER_H

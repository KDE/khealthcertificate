/*
 * SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef JWKLOADER_P_H
#define JWKLOADER_P_H

#include "opensslpp_p.h"

class QJsonObject;
class QString;

/** Load JSON Web Keys for use with OpenSSL. */
class JwkLoader
{
public:
    static openssl::evp_pkey_ptr loadPublicKey(const QString &fileName);
    static openssl::evp_pkey_ptr loadPublicKey(const QJsonObject &keyObj);
};

#endif // JWKLOADER_P_H

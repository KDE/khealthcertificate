/*
 * SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "jwkloader_p.h"
#include "logging.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>

#include <openssl/bn.h>
#include <openssl/obj_mac.h>

openssl::evp_pkey_ptr JwkLoader::loadPublicKey(const QString &fileName)
{
    QFile f(fileName);
    if (!f.open(QFile::ReadOnly)) {
        qCWarning(Log) << f.errorString();
        return openssl::evp_pkey_ptr(nullptr, &EVP_PKEY_free);
    }

    return loadPublicKey(QJsonDocument::fromJson(f.readAll()).object());
}

openssl::evp_pkey_ptr JwkLoader::loadPublicKey(const QJsonObject &keyObj)
{
    auto evp = openssl::evp_pkey_ptr(nullptr, &EVP_PKEY_free);

    const auto kty = keyObj.value(QLatin1String("kty")).toString();
    if (kty == QLatin1String("EC")) {
        auto ecKey = openssl::ec_key_ptr(nullptr, &EC_KEY_free);
        const auto crv = keyObj.value(QLatin1String("crv")).toString();
        if (crv == QLatin1String("P-256")) {
            ecKey = openssl::ec_key_ptr(EC_KEY_new_by_curve_name(NID_X9_62_prime256v1), &EC_KEY_free);
        } else if (crv == QLatin1String("P-384")) {
            ecKey = openssl::ec_key_ptr(EC_KEY_new_by_curve_name(NID_secp384r1), &EC_KEY_free);
        } else if (crv == QLatin1String("P-521")) {
            ecKey = openssl::ec_key_ptr(EC_KEY_new_by_curve_name(NID_secp521r1), &EC_KEY_free);
        } else {
            qCWarning(Log) << "Unsupported curve type" << crv;
            return evp;
        }

        const auto xData = QByteArray::fromBase64(keyObj.value(QLatin1String("x")).toString().toUtf8(), QByteArray::Base64UrlEncoding);
        const auto x = BN_bin2bn(reinterpret_cast<const uint8_t*>(xData.constData()), xData.size(), nullptr);
        const auto yData = QByteArray::fromBase64(keyObj.value(QLatin1String("y")).toString().toUtf8(), QByteArray::Base64UrlEncoding);
        const auto y = BN_bin2bn(reinterpret_cast<const uint8_t*>(yData.constData()), xData.size(), nullptr);
        EC_KEY_set_public_key_affine_coordinates(ecKey.get(), x, y);

        evp.reset(EVP_PKEY_new());
        EVP_PKEY_assign_EC_KEY(evp.get(), ecKey.release());
    } else {
        qCWarning(Log) << "unsuporrted key type:" << kty;
    }

    return evp;
}

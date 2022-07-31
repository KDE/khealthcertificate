/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "x509loader_p.h"

#include <QByteArray>

openssl::x509_ptr X509Loader::readFromDER(const QByteArray &data)
{
    const uint8_t *certData = reinterpret_cast<const uint8_t*>(data.constData());
    return openssl::x509_ptr(d2i_X509(nullptr, &certData, data.size()), &X509_free);
}

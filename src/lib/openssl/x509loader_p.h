/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef X509_P_H
#define X509_P_H

#include "opensslpp_p.h"

class QByteArray;

/** X.509 helper functions. */
namespace X509Loader
{
    openssl::x509_ptr readFromDER(const QByteArray &data);
}

#endif // X509_H

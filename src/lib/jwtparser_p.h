/*
 * SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef JWTPARSER_P_H
#define JWTPARSER_P_H

#include "khealthcertificate.h"

#include <QJsonObject>

/** Decoding of JSON Web Tokens (JWT). */
class JwtParser
{
public:
    JwtParser();
    ~JwtParser();

    void parse(const QByteArray &data);

    QJsonObject payload() const;
    KHealthCertificate::SignatureValidation signatureState() const;

private:
    QJsonObject m_payload;
    KHealthCertificate::SignatureValidation m_signatureState = KHealthCertificate::InvalidSignature;
};

#endif // JWTPARSER_P_H

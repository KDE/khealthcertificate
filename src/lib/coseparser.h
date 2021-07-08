/*
 * SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef COSEPARSER_H
#define COSEPARSER_H

#include "opensslpp.h"

#include <QByteArray>

/** Parser for CBOR Object Signing and Encryption (COSE) data.
 *  @see RFC 8152
 */
class CoseParser
{
public:
    void parse(const QByteArray &data);
    /** The signed content. */
    QByteArray payload() const;

    enum SignatureState {
        Unknown,
        UnknownCertificate,
        UnsupportedAlgorithm,
        InvalidSignature,
        ValidSignature,
    };
    /** Result of validating the COSE signature. */
    SignatureState signatureState() const;

private:
    void clear();
    void validateECDSA(const openssl::evp_pkey_ptr &pkey, int algorithm);
    // the raw data that is being signed, see RFC 8152 § 4.4
    QByteArray sigStructure() const;

    QByteArray m_protectedParams;
    QByteArray m_payload;
    QByteArray m_signature;
    QByteArray m_kid;
    SignatureState m_signatureState = Unknown;
};

#endif // COSEPARSER_H

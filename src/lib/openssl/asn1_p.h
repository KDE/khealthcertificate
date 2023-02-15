/*
 * SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KHEALTHCERTIFICATE_ASN1_P_H
#define KHEALTHCERTIFICATE_ASN1_P_H

#include "config-openssl_p.h"
#include "opensslpp_p.h"

#include <QByteArray>

#include <openssl/asn1.h>

#include <cstdint>

namespace ASN1 {

class Object
{
public:
    explicit inline Object(const uint8_t *begin, const uint8_t *end)
        : m_begin(begin)
        , m_contentBegin(begin)
        , m_outerEnd(end)
    {
        long length = 0;
        ASN1_get_object(&m_contentBegin, &length, &m_tag, &m_class, std::distance(begin, end));
        m_end = m_contentBegin + length;
    }

    explicit inline Object(const char *begin, const char *end)
        : Object(reinterpret_cast<const uint8_t*>(begin), reinterpret_cast<const uint8_t*>(end)) {}

    inline int tag() const { return m_tag; }
    inline const char* tagName() const { return ASN1_tag2str(m_tag); }
    inline int asn1Class() const { return m_class; }

    inline const uint8_t* begin() const { return m_begin; }
    inline const uint8_t* contentBegin() const { return m_contentBegin; }
    inline const uint8_t* end() const { return m_end; }

    inline auto size() const { return std::distance(m_begin, m_end); }
    inline auto contentSize() const { return std::distance(m_contentBegin, m_end); }

    inline Object firstChild() const { return Object(m_contentBegin, m_end); }
    inline bool hasNext() const { return m_end < m_outerEnd && m_tag != V_ASN1_EOC; }
    inline Object next() const { return Object(end(), m_outerEnd); }

    inline int64_t readInt64() const
    {
        if (tag() != V_ASN1_INTEGER) {
            return {};
        }
        auto it = begin();
        const auto ai = openssl::asn1_integer_ptr(d2i_ASN1_INTEGER(nullptr, &it, size()));
        int64_t result = 0;
#if HAVE_LIBRESSL
        const auto bn = readBignum();
        if (BN_num_bytes(bn.get()) > (int)sizeof(result)) {
            return {};
        }
        BN_bn2binpad(bn.get(), reinterpret_cast<uint8_t*>(&result), sizeof(result));
        return qFromBigEndian((qint64)result);
#else
        ASN1_INTEGER_get_int64(&result, ai.get());
        return result;
#endif
    }

    inline openssl::bn_ptr readBignum() const
    {
        if (tag() != V_ASN1_INTEGER) {
            return {};
        }
        auto it = begin();
        const auto ai = openssl::asn1_integer_ptr(d2i_ASN1_INTEGER(nullptr, &it, size()));
        return openssl::bn_ptr(ASN1_INTEGER_to_BN(ai.get(), nullptr));
    }

    inline QByteArray readOctetString() const
    {
        if (tag() != V_ASN1_OCTET_STRING) {
            return {};
        }
        auto it = begin();
        const auto os = openssl::asn1_octet_string_ptr(d2i_ASN1_OCTET_STRING(nullptr, &it, size()));
        return QByteArray(reinterpret_cast<const char*>(os->data), os->length);
    }

    inline QByteArray readPrintableString() const
    {
        if (tag() != V_ASN1_PRINTABLESTRING) {
            return {};
        }
        auto it = begin();
        const auto ps = openssl::asn1_printable_string_ptr(d2i_ASN1_PRINTABLESTRING(nullptr, &it, size()));
        return QByteArray(reinterpret_cast<const char*>(ps->data), ps->length);
    }
private:
    int m_tag = 0;
    int m_class = 0;
    const uint8_t *m_begin = nullptr;
    const uint8_t *m_contentBegin = nullptr;
    const uint8_t *m_end = nullptr;
    const uint8_t *m_outerEnd = nullptr;
};

}

#endif

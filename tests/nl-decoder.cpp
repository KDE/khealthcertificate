/*
 * SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "../src/lib/opensslpp_p.h"

#include <QDebug>
#include <QFile>

// creative variation of Base45...
static constexpr const char dutchBase45Table[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ $%*+-./:";

static uint8_t dutchBase45MapFromChar(char c)
{
    const auto it = std::find(std::begin(dutchBase45Table), std::end(dutchBase45Table), c);
    if (it == std::end(dutchBase45Table)) {
        qWarning() << "invalid base45 character:" << c;
        return 0;
    }
    return std::distance(std::begin(dutchBase45Table), it);
}

static QByteArray dutchBase45Decode(const QByteArray &in)
{
    openssl::bn_ptr bn(BN_new(), &BN_free);
    BN_zero(bn.get());
    for (int i = 0; i < in.size(); ++i) {
        BN_mul_word(bn.get(), 45);
        BN_add_word(bn.get(), dutchBase45MapFromChar(in[i]));
    }

    QByteArray out;
    out.resize(BN_num_bytes(bn.get()));
    BN_bn2bin(bn.get(), reinterpret_cast<uint8_t*>(out.data()));

    return out;
}

// creative string encoding...
static QByteArray decodeByteArray(const uint8_t *&it, std::size_t length)
{
    auto ai = openssl::asn1_integer_ptr(d2i_ASN1_INTEGER(nullptr, &it, length), &ASN1_INTEGER_free);

    auto bn = openssl::bn_ptr(BN_new(), &BN_free);
    BN_zero(bn.get());
    for (auto i = 0; i < ai->length; ++i) {
        BN_mul_word(bn.get(), 1 << 8);
        BN_add_word(bn.get(), ai->data[i]);
    }
    BN_div_word(bn.get(), 2);

    QByteArray s;
    s.resize(BN_num_bytes(bn.get()));
    BN_bn2bin(bn.get(), reinterpret_cast<uint8_t*>(s.data()));

    return s;
}

static void dumpMetaData(const QByteArray &md)
{
    int64_t length = 0;
    int tag = 0, asn1Class = 0;
    auto it = reinterpret_cast<const uint8_t*>(md.constData());
    const auto endIt = it + md.size();
    ASN1_get_object(&it, &length, &tag, &asn1Class, md.size());
//     qDebug() << length << tag << asn1Class << ASN1_tag2str(tag);

    while (it != endIt) {
        auto it2 = it;
        ASN1_get_object(&it, &length, &tag, &asn1Class, std::distance(it, endIt));
        if (tag == V_ASN1_EOC && asn1Class == 0x0) {
            break;
        }
//         qDebug() << length << tag << asn1Class << ASN1_tag2str(tag);
        if (tag == V_ASN1_OCTET_STRING) {
            auto os = openssl::asn1_octet_string_ptr(d2i_ASN1_OCTET_STRING(nullptr, &it2, length + std::distance(it2, it)), &ASN1_PRINTABLESTRING_free);
            qDebug() << "version:" << QByteArray((const char*)os->data, os->length);
            it = it2;
        } else if (tag == V_ASN1_PRINTABLESTRING) {
            auto ps = openssl::asn1_printable_string_ptr(d2i_ASN1_PRINTABLESTRING(nullptr, &it2, length + std::distance(it2, it)), &ASN1_PRINTABLESTRING_free);
            qDebug() << "issuer key:" << QByteArray((const char*)ps->data, ps->length);
            it = it2;
        } else {
            it += length;
        }
    }
}

static const char *field_names[] = {
    "isSpecimen",
    "isPaperProof",
    "validFrom",
    "validForHours",
    "firstNameInitial",
    "lastNameInitial",
    "birthDay",
    "birthMonth"
};

static void dumpInnerSequence(const uint8_t *&it, const uint8_t *endIt)
{
    int64_t length = 0;
    int tag = 0, asn1Class = 0;

    // metadata field
    auto it2 = it;
    ASN1_get_object(&it, &length, &tag, &asn1Class, std::distance(it, endIt));
    dumpMetaData(decodeByteArray(it2, length + std::distance(it2, it)));
    it = it2;

    // string fields
    int i = 0;
    while (it != endIt) {
        it2 = it;
        ASN1_get_object(&it, &length, &tag, &asn1Class, std::distance(it, endIt));
        if (tag == V_ASN1_EOC && asn1Class == 0x0) {
            break;
        }
//         qDebug() << length << tag << asn1Class << ASN1_tag2str(tag);
        qDebug() << field_names[i++] << decodeByteArray(it2, length + std::distance(it2, it));
        it = it2;
    }
}

static void dumpOuterSequence(const uint8_t *&it, const uint8_t *endIt)
{
    int64_t length = 0;
    int tag = 0, asn1Class = 0;
    while (it != endIt) {
        ASN1_get_object(&it, &length, &tag, &asn1Class, std::distance(it, endIt));
        if (tag == V_ASN1_EOC && asn1Class == 0x0) {
            break;
        }
        qDebug() << length << tag << asn1Class << ASN1_tag2str(tag);
        if (tag == V_ASN1_SEQUENCE) {
            dumpInnerSequence(it, it + length);
        } else {
            // TODO these fields have to be the signature
            it += length;
        }
    }
}

int main(int argc, char **argv)
{
    QFile f;
    f.open(stdin, QFile::ReadOnly);
    const auto in = f.readAll();

    // strip prefix
    if (!in.startsWith("NL2:")) {
        return 1;
    }

    // "base45" decode
    const auto base45Decoded = dutchBase45Decode(in.mid(4));
//     qDebug() << base45Decoded;

    int64_t length = 0;
    int tag = 0, asn1Class = 0;
    auto it = reinterpret_cast<const uint8_t*>(base45Decoded.constData());
    ASN1_get_object(&it, &length, &tag, &asn1Class, base45Decoded.size());
    qDebug() << length << tag << asn1Class << ASN1_tag2str(tag);
    dumpOuterSequence(it, it + length);

    return 0;
}

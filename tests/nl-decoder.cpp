/*
 * SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "../src/lib/nl-coronacheck/nlbase45.cpp"
#include "../src/lib/openssl/opensslpp_p.h"

#include <QDebug>
#include <QFile>

// creative string encoding...
static QByteArray decodeByteArray(const uint8_t *&it, std::size_t length)
{
    auto ai = openssl::asn1_integer_ptr(d2i_ASN1_INTEGER(nullptr, &it, length));

    auto bn = openssl::bn_ptr(BN_new());
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
    long int length = 0;
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
            auto os = openssl::asn1_octet_string_ptr(d2i_ASN1_OCTET_STRING(nullptr, &it2, length + std::distance(it2, it)));
            qDebug() << "version:" << QByteArray((const char*)os->data, os->length);
            it = it2;
        } else if (tag == V_ASN1_PRINTABLESTRING) {
            auto ps = openssl::asn1_printable_string_ptr(d2i_ASN1_PRINTABLESTRING(nullptr, &it2, length + std::distance(it2, it)));
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
    long int length = 0;
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
    long int length = 0;
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
    const auto base45Decoded = NLBase45::decode(in.begin() + 4, in.end());
//     qDebug() << base45Decoded;

    long int length = 0;
    int tag = 0, asn1Class = 0;
    auto it = reinterpret_cast<const uint8_t*>(base45Decoded.constData());
    ASN1_get_object(&it, &length, &tag, &asn1Class, base45Decoded.size());
    qDebug() << length << tag << asn1Class << ASN1_tag2str(tag);
    dumpOuterSequence(it, it + length);

    return 0;
}

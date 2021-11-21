/*
 * SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "nlcoronacheckparser_p.h"
#include "nlbase45_p.h"
#include "logging.h"

#include "openssl/asn1_p.h"

#include <KHealthCertificate/KTestCertificate>

#include <QDebug>
#include <QVariant>

static QByteArray nlDecodeAsn1ByteArray(const ASN1::Object &obj)
{
    auto it = obj.begin();
    auto ai = openssl::asn1_integer_ptr(d2i_ASN1_INTEGER(nullptr, &it, obj.size()), &ASN1_INTEGER_free);
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

QVariant NLCoronaCheckParser::parse(const QByteArray &data)
{
    if (!data.startsWith("NL2:") || data.size() < 5) {
        return {};
    }
    const auto rawData = NLBase45::decode(data.begin() + 4, data.end());

    const auto root = ASN1::Object(rawData.begin(), rawData.end());
    if (root.tag() != V_ASN1_SEQUENCE) {
        qCWarning(Log) << "wrong ASN1 root node type" << root.tagName();
        return {};
    }

    // TODO signature verification
    // DisclosureTimeSeconds: int
    // C: int
    // A: int
    // EResponse: int
    // VResponse: int
    // AResponse: int
    // ADisclosed: int[]
    auto outer = root.firstChild();
    outer = outer.next();
    outer = outer.next();
    outer = outer.next();
    outer = outer.next();
    outer = outer.next();
    outer = outer.next();
    if (outer.tag() != V_ASN1_SEQUENCE) {
        qCWarning(Log) << "wrong ADisclosed field type" << outer.tagName();
        return {};
    }

    // metadata
    // isSpecimen
    // isPaperProof
    // validFrom
    // validForHours
    // firstNameInitial
    // lastNameInitial
    // birthDay
    // birthMonth
    auto adisclosed = outer.firstChild();

    // metadata: byte array containing another ASN1 sequence
    // version: OCTET STRING
    // issuer key id: PRINTABLESTRING
    const auto rawMetaData = nlDecodeAsn1ByteArray(adisclosed);
    const auto metadata = ASN1::Object(rawMetaData.begin(), rawMetaData.end());
    if (metadata.tag() != V_ASN1_SEQUENCE) {
        qCWarning(Log) << "meta data is not a ASN.1 SEQUENCE:" << metadata.tagName();
        return {};
    }
    auto metadataEntry = metadata.firstChild();
    const auto version = metadataEntry.readOctetString();
    if (version.size() != 1 || version[0] != 0x02) {
        qCWarning(Log) << "unsupported version:" << version;
        return {};
    }
    metadataEntry = metadataEntry.next();
    const auto issuer = metadataEntry.readPrintableString();

    // isSpecimen invalidate the certificate state
    adisclosed = adisclosed.next();
    const auto rawIsSpecimen = nlDecodeAsn1ByteArray(adisclosed);
    const bool isSpecimen = rawIsSpecimen.size() != 1 || rawIsSpecimen[0] != '0';
    adisclosed = adisclosed.next();

    // valid time range
    adisclosed = adisclosed.next();
    const auto validFrom = QDateTime::fromSecsSinceEpoch(nlDecodeAsn1ByteArray(adisclosed).toLongLong());
    adisclosed = adisclosed.next();
    const auto validTo = validFrom.addSecs(3600 * nlDecodeAsn1ByteArray(adisclosed).toInt());

    // name
    adisclosed = adisclosed.next();
    auto name = QString::fromUtf8(nlDecodeAsn1ByteArray(adisclosed));
    adisclosed = adisclosed.next();
    name += QLatin1Char(' ') + QString::fromUtf8(nlDecodeAsn1ByteArray(adisclosed));

    // birthday
    adisclosed = adisclosed.next();
    auto bd = QString::fromUtf8(nlDecodeAsn1ByteArray(adisclosed));
    if (!adisclosed.hasNext()) {
        qCWarning(Log) << "ADisclosed sequence too short";
        return {};
    }
    adisclosed = adisclosed.next();
    bd += QString::fromUtf8(nlDecodeAsn1ByteArray(adisclosed));
    const auto birthday = QDate::fromString(bd, QStringLiteral("dM"));

    KTestCertificate cert;
    cert.setCountry(QStringLiteral("NL"));
    cert.setDisease(QStringLiteral("COVID-19"));
    cert.setResult(KTestCertificate::Negative);
    cert.setName(name);
    cert.setDateOfBirth(birthday);
    cert.setCertificateIssueDate(validFrom);
    cert.setCertificateExpiryDate(validTo);
    cert.setCertificateIssuer(QString::fromUtf8(issuer)); // ### temporary
    cert.setRawData(data);
    cert.setSignatureState(isSpecimen ? KHealthCertificate::InvalidSignature : KHealthCertificate::UncheckedSignature); // TODO

    return cert;
}

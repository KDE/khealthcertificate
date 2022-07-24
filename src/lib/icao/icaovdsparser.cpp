/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "icaovdsparser_p.h"
#include "logging.h"

#include <openssl/opensslpp_p.h>
#include <openssl/verify_p.h>

#include <KHealthCertificate/KTestCertificate>
#include <KHealthCertificate/KVaccinationCertificate>

#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

void IcaoVdsParser::init()
{
    Q_INIT_RESOURCE(icaovds_data);
}

template <typename Cert>
static void parsePersonalInformation(Cert &cert, const QJsonObject &pidObj)
{
    cert.setName(pidObj.value(QLatin1String("n")).toString());
    cert.setDateOfBirth(QDate::fromString(pidObj.value(QLatin1String("dob")).toString(), Qt::ISODate));
}

static int jsonValueToInt(const QJsonValue &v)
{
    if (v.isDouble()) {
        return v.toInt();
    }
    if (v.isString()) {
        return v.toString().toInt();
    }
    return 0;
}

static QString lookupDisease(const QString &code)
{
    QFile f(QLatin1String(":/org.kde.khealthcertificate/icao/diseases.json"));
    if (!f.open(QFile::ReadOnly)) {
        qCWarning(Log) << f.fileName() << f.errorString();
        return code;
    }

    const auto obj = QJsonDocument::fromJson(f.readAll()).object();
    const auto name = obj.value(code.left(4)).toString();
    return name.isEmpty() ? code : name;
}

static QString lookupVaccine(const QString &code)
{
    QFile f(QLatin1String(":/org.kde.khealthcertificate/icao/vaccines.json"));
    if (!f.open(QFile::ReadOnly)) {
        qCWarning(Log) << f.fileName() << f.errorString();
        return code;
    }

    const auto obj = QJsonDocument::fromJson(f.readAll()).object();
    const auto name = obj.value(code).toString();
    return name.isEmpty() ? code : name;
}

QVariant IcaoVdsParser::parse(const QByteArray &data)
{
    const auto doc = QJsonDocument::fromJson(data);

    QJsonObject rootObj;
    if (doc.isObject()) {
        rootObj = doc.object();
    } else if (doc.isArray() && doc.array().size() == 1) {
        rootObj = doc.array().at(0).toObject(); // TODO multiple entries?
    }

    const auto dataObj = rootObj.value(QLatin1String("data")).toObject();
    const auto hdrObj = dataObj.value(QLatin1String("hdr")).toObject();
    const auto msgObj = dataObj.value(QLatin1String("msg")).toObject();

    if (hdrObj.value(QLatin1String("v")).toInt() != 1) {
        return {};
    }

    const auto sigObj = rootObj.value(QLatin1String("sig")).toObject();
    const auto cert = QByteArray::fromBase64(sigObj.value(QLatin1String("cer")).toString().toUtf8(), QByteArray::Base64UrlEncoding);
    const uint8_t *certData = reinterpret_cast<const uint8_t*>(cert.data());
    const openssl::x509_ptr x509Cert(d2i_X509(nullptr, &certData, cert.size()), &X509_free);
    const openssl::evp_pkey_ptr pkey(X509_get_pubkey(x509Cert.get()), &EVP_PKEY_free);
    // TODO we need to verify that certificate is actually trusted!

    const auto alg = sigObj.value(QLatin1String("alg")).toString();
    const auto signature = QByteArray::fromBase64(sigObj.value(QLatin1String("sigvl")).toString().toUtf8(), QByteArray::Base64UrlEncoding);

    // this need RFC 8785 JSON canonicalization
    const auto signedData = QJsonDocument(dataObj).toJson(QJsonDocument::Compact);

    bool valid = false;
    if (alg == QLatin1String("ES256")) {
        valid = Verify::verifyECDSA(pkey, EVP_sha256(), signedData.constData(), signedData.size(), signature.constData(), signature.size());
    } else if (alg == QLatin1String("ES384")) {
        valid = Verify::verifyECDSA(pkey, EVP_sha384(), signedData.constData(), signedData.size(), signature.constData(), signature.size());
    } else if (alg == QLatin1String("ES512")) {
        valid = Verify::verifyECDSA(pkey, EVP_sha512(), signedData.constData(), signedData.size(), signature.constData(), signature.size());
    } else {
        qCWarning(Log) << "signature algorithm not supported:" << alg;
    }

    const auto type = hdrObj.value(QLatin1String("t")).toString();
    if (type == QLatin1String("icao.vacc")) {
        KVaccinationCertificate cert;
        parsePersonalInformation(cert, msgObj.value(QLatin1String("pid")).toObject());
        cert.setCertificateId(msgObj.value(QLatin1String("uvci")).toString());

        const auto veArray = msgObj.value(QLatin1String("ve")).toArray();
        if (veArray.isEmpty()) {
            return {};
        }
        const auto veObj = veArray.at(0).toObject(); // TODO multiple entries?
        cert.setVaccineType(lookupVaccine(veObj.value(QLatin1String("des")).toString()));
        cert.setDisease(lookupDisease(veObj.value(QLatin1String("dis")).toString()));
        cert.setVaccine(veObj.value(QLatin1String("nam")).toString());

        const auto vdArray = veObj.value(QLatin1String("vd")).toArray();
        for (const auto &vdVal : vdArray) {
            const auto vdObj = vdVal.toObject();
            const auto seq = jsonValueToInt(vdObj.value(QLatin1String("seq")));
            if (seq < cert.dose()) {
                continue;
            }
            cert.setDose(seq);
            cert.setDate(QDate::fromString(vdObj.value(QLatin1String("dvc")).toString(), Qt::ISODate));
            cert.setCountry(vdObj.value(QLatin1String("ctr")).toString());
        }

        cert.setRawData(data);
        cert.setSignatureState(valid ? KHealthCertificate::UncheckedSignature : KHealthCertificate::InvalidSignature); // TODO
        return cert;
    }

    if (type == QLatin1String("icao.test")) {
        KTestCertificate cert;
        parsePersonalInformation(cert, msgObj.value(QLatin1String("pid")).toObject());
        cert.setCertificateId(msgObj.value(QLatin1String("utci")).toString());

        const auto spObj = msgObj.value(QLatin1String("sp")).toObject();
        cert.setTestCenter(spObj.value(QLatin1String("spn")).toString());
        cert.setCountry(spObj.value(QLatin1String("ctr")).toString());

        const auto datObj = msgObj.value(QLatin1String("dat")).toObject();
        cert.setDate(QDateTime::fromString(datObj.value(QLatin1String("sc")).toString(), Qt::ISODate).date());

        const auto trObj = msgObj.value(QLatin1String("tr")).toObject();
        cert.setTestType(trObj.value(QLatin1String("tc")).toString());
        const auto result = trObj.value(QLatin1String("r")).toString();
        cert.setResultString(result);
        if (result.compare(QLatin1String("negative"), Qt::CaseInsensitive) == 0) {
            cert.setResult(KTestCertificate::Negative);
        } else if (result.compare(QLatin1String("positive"), Qt::CaseInsensitive) == 0) {
            cert.setResult(KTestCertificate::Positive);
        } else {
            cert.setResult(KTestCertificate::Unknown);
        }

        cert.setRawData(data);
        cert.setSignatureState(valid ? KHealthCertificate::UncheckedSignature : KHealthCertificate::InvalidSignature); // TODO
        return cert;
    }

    return {};
}

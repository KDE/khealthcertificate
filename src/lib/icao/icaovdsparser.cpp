/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "icaovdsparser_p.h"
#include "logging.h"

#include <openssl/opensslpp_p.h>
#include <openssl/verify_p.h>
#include <openssl/x509loader_p.h>

#include <openssl/x509v3.h>

#include <KHealthCertificate/KTestCertificate>
#include <KHealthCertificate/KVaccinationCertificate>

#include <KCountry>

#include <QDirIterator>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

void IcaoVdsParser::init()
{
    Q_INIT_RESOURCE(icao_csca_certs);
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
    QFile f(QLatin1String(":/org.kde.khealthcertificate/icao/data/diseases.json"));
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
    QFile f(QLatin1String(":/org.kde.khealthcertificate/icao/data/vaccines.json"));
    if (!f.open(QFile::ReadOnly)) {
        qCWarning(Log) << f.fileName() << f.errorString();
        return code;
    }

    const auto obj = QJsonDocument::fromJson(f.readAll()).object();
    const auto name = obj.value(code).toString();
    return name.isEmpty() ? code : name;
}

static QString alpha3ToAlpha2(const QString &alpha3)
{
    const auto c = KCountry::fromAlpha3(alpha3);
    return c.isValid() ? c.alpha2() : alpha3;
}

static KHealthCertificate::SignatureValidation verifyCertificate(const openssl::x509_ptr &x509Cert)
{
    const auto keyId = X509_get0_authority_key_id(x509Cert.get());
    if (!keyId) {
        return KHealthCertificate::InvalidSignature;
    }
    const auto keyIdStr = QString::fromUtf8(QByteArray(reinterpret_cast<const char*>(keyId->data), keyId->length).toHex());
    qCDebug(Log) << keyIdStr;

    // single certificate file for keyId
    QFile issuerCertFile(QLatin1String(":/org.kde.khealthcertificate/icao/certs/") + keyIdStr + QLatin1String(".der"));
    if (issuerCertFile.open(QFile::ReadOnly)) {
        const auto x509IssuerCert = X509Loader::readFromDER(issuerCertFile.readAll());
        const openssl::evp_pkey_ptr issuerPkey(X509_get_pubkey(x509IssuerCert.get()));
        const auto certValid = X509_verify(x509Cert.get(), issuerPkey.get());
        if (certValid == 1) {
            return KHealthCertificate::UncheckedSignature;
        }
        return KHealthCertificate::InvalidSignature;
    }

    // multiple certificates for keyId, try all of them
    bool foundInvalid = false;
    for (QDirIterator it(QLatin1String(":/org.kde.khealthcertificate/icao/certs/") + keyIdStr, QDir::Files); it.hasNext();) {
        QFile issuerCertFile(it.next());
        if (issuerCertFile.open(QFile::ReadOnly)) {
            const auto x509IssuerCert = X509Loader::readFromDER(issuerCertFile.readAll());
            const openssl::evp_pkey_ptr issuerPkey(X509_get_pubkey(x509IssuerCert.get()));
            const auto certValid = X509_verify(x509Cert.get(), issuerPkey.get());
            if (certValid == 1) {
                return KHealthCertificate::UncheckedSignature;
            }
            foundInvalid = true;
        } else {
            qCWarning(Log) << issuerCertFile.fileName() << issuerCertFile.errorString();
        }
    }

    if (!foundInvalid) {
        qCWarning(Log) << "No CSCA certificate found for key id" << keyId;
    }
    return foundInvalid ? KHealthCertificate::InvalidSignature : KHealthCertificate::UnknownSignature;
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

    // verify certificate used for the signature
    const auto cert = QByteArray::fromBase64(sigObj.value(QLatin1String("cer")).toString().toUtf8(), QByteArray::Base64UrlEncoding);
    const uint8_t *certData = reinterpret_cast<const uint8_t*>(cert.data());
    const openssl::x509_ptr x509Cert(d2i_X509(nullptr, &certData, cert.size()));
    KHealthCertificate::SignatureValidation sigState = verifyCertificate(x509Cert);

    // verify that the content signature is correct
    const openssl::evp_pkey_ptr pkey(X509_get_pubkey(x509Cert.get()));
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
    if (valid && sigState == KHealthCertificate::UncheckedSignature) {
        sigState = KHealthCertificate::ValidSignature;
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
        for (const auto &veVal : veArray) {
            const auto veObj = veVal.toObject();
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
                cert.setCountry(alpha3ToAlpha2(vdObj.value(QLatin1String("ctr")).toString()));
            }
        }

        const auto compactData = doc.toJson(QJsonDocument::Compact);
        cert.setRawData(compactData.size() < data.size() ? compactData : data);
        cert.setSignatureState(sigState);
        return cert;
    }

    if (type == QLatin1String("icao.test")) {
        KTestCertificate cert;
        parsePersonalInformation(cert, msgObj.value(QLatin1String("pid")).toObject());
        cert.setCertificateId(msgObj.value(QLatin1String("utci")).toString());

        const auto spObj = msgObj.value(QLatin1String("sp")).toObject();
        cert.setTestCenter(spObj.value(QLatin1String("spn")).toString());
        cert.setCountry(alpha3ToAlpha2(spObj.value(QLatin1String("ctr")).toString()));

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

        const auto compactData = doc.toJson(QJsonDocument::Compact);
        cert.setRawData(compactData.size() < data.size() ? compactData : data);
        cert.setSignatureState(sigState);
        return cert;
    }

    return {};
}

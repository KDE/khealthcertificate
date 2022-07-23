/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "icaovdsparser_p.h"
#include "logging.h"

#include <KHealthCertificate/KTestCertificate>
#include <KHealthCertificate/KVaccinationCertificate>

#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

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
        cert.setVaccineType(veObj.value(QLatin1String("des")).toString()); // TODO WHO ICD-11 lookup
        cert.setDisease(veObj.value(QLatin1String("dis")).toString()); // TODO WHO ICD-11 lookup
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
        cert.setSignatureState(KHealthCertificate::UncheckedSignature); // TODO
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
        cert.setSignatureState(KHealthCertificate::UncheckedSignature); // TODO
        return cert;
    }

    return {};
}

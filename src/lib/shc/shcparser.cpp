/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "shcparser_p.h"
#include "jwtparser_p.h"
#include "kvaccinationcertificate.h"
#include "logging.h"

#include <QByteArray>
#include <QDebug>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QVariant>

void ShcParser::init()
{
    Q_INIT_RESOURCE(shc_certs);
    Q_INIT_RESOURCE(shc_certs_manual);
    Q_INIT_RESOURCE(shc_data);
}

QVariant ShcParser::parse(const QByteArray &data)
{
    if (!data.startsWith("shc:/")) {
        return {};
    }

    if (data.indexOf('/', 5) > 0) {
        qCWarning(Log) << "SHC chunked data not supported yet!";
        return {};
    }

    QByteArray unpacked;
    unpacked.reserve(data.size() / 2);
    for (int i = 5; i < data.size() - 1; i += 2) {
        unpacked.push_back((data[i] - '0') * 10 + (data[i + 1] - '0') + 45);
    }

    JwtParser jwt;
    jwt.parse(unpacked);

    const auto nbf = QDateTime::fromSecsSinceEpoch(jwt.payload().value(QLatin1String("nbf")).toDouble());
    const auto vc = jwt.payload().value(QLatin1String("vc")).toObject();
    const auto types = vc.value(QLatin1String("type")).toArray();
    //qDebug().noquote() << QJsonDocument(vc).toJson();
    for (const auto &type : types) {
        if (type.toString() == QLatin1String("https://smarthealth.cards#immunization")) {
            auto cert = parseImmunization(vc.value(QLatin1String("credentialSubject")).toObject());
            cert.setCertificateIssueDate(nbf);
            cert.setCertificateIssuer(jwt.payload().value(QLatin1String("iss")).toString());
            cert.setRawData(data);
            cert.setSignatureState(jwt.signatureState());
            return cert;
        }
    }

    return {};
}

KVaccinationCertificate ShcParser::parseImmunization(const QJsonObject &obj)
{
    const auto entries = obj.value(QLatin1String("fhirBundle")).toObject().value(QLatin1String("entry")).toArray();
    KVaccinationCertificate cert;
    for (const auto &entryV : entries) {
        const auto res = entryV.toObject().value(QLatin1String("resource")).toObject();
        const auto resourceType = res.value(QLatin1String("resourceType")).toString();
        if (resourceType == QLatin1String("Patient")) {
            cert.setDateOfBirth(QDate::fromString(res.value(QLatin1String("birthDate")).toString(), Qt::ISODate));
            const auto nameArray = res.value(QLatin1String("name")).toArray();
            if (nameArray.size() != 1) {
                return {};
            }
            const auto nameObj = nameArray.at(0).toObject();
            const auto given = nameObj.value(QLatin1String("given")).toArray();
            QStringList nameParts;
            nameParts.reserve(given.size() + 1);
            for (const auto &givenV : given) {
                nameParts.push_back(givenV.toString());
            }
            nameParts.push_back(nameObj.value(QLatin1String("family")).toString());
            cert.setName(nameParts.join(QLatin1Char(' ')));
        }
        else if (resourceType == QLatin1String("Immunization")) {
            if (res.value(QLatin1String("status")).toString() != QLatin1String("completed")) {
                continue;
            }
            const auto dt = QDate::fromString(res.value(QLatin1String("occurrenceDateTime")).toString(), Qt::ISODate);
            if (cert.date().isValid() && cert.date() > dt) { // TODO alternatively, emit two certs, one for each dose?
                cert.setDose(cert.dose() + 1);
                continue;
            }

            cert.setDate(dt);
            cert.setDose(std::max(1, cert.dose() + 1));

            const auto vacCode = res.value(QLatin1String("vaccineCode")).toObject().value(QLatin1String("coding")).toArray();
            if (vacCode.size() != 1) {
                continue;
            }
            const auto vacObj = vacCode.at(0).toObject();
            QJsonObject cvxData;
            if (vacObj.value(QLatin1String("system")).toString() == QLatin1String("http://hl7.org/fhir/sid/cvx")) {
                QFile cvxFile(QStringLiteral(":/org.kde.khealthcertificate/shc/hl7-cvx-codes.json"));
                if (!cvxFile.open(QFile::ReadOnly)) {
                    qCWarning(Log) << cvxFile.errorString();
                }
                const auto cvxDb = QJsonDocument::fromJson(cvxFile.readAll()).object();
                cvxData = cvxDb.value(vacObj.value(QLatin1String("code")).toString()).toObject();
            }

            if (cvxData.isEmpty()) {
                cert.setVaccine(vacObj.value(QLatin1String("system")).toString() + QLatin1Char('/') + vacObj.value(QLatin1String("code")).toString());
            } else {
                cert.setVaccine(cvxData.value(QLatin1String("n")).toString());
                cert.setDisease(cvxData.value(QLatin1String("d")).toString());
                cert.setManufacturer(cvxData.value(QLatin1String("m")).toString());
            }
        }
        else {
            qCDebug(Log) << "unhandled resource type:" << resourceType << res;
        }
    }
    return cert;
}

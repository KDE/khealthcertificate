/*
 * SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "verifiablecredentialparser_p.h"
#include "kvaccinationcertificate.h"

#include <QByteArray>
#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QVariant>

QVariant VerifiableCredentialParser::parse(const QByteArray &data)
{
    QJsonParseError error;
    const auto doc = QJsonDocument::fromJson(data, &error);
    if (error.error != QJsonParseError::NoError) {
        return {};
    }

    // TODO check this is actually a VerifiableCredential structure
    // TODO check the types on the subobjects we use

    KVaccinationCertificate cert;
    const auto subject = doc.object().value(QLatin1String("credentialSubject")).toObject();
    cert.setName(subject.value(QLatin1String("name")).toString());
    const auto evidences = doc.object().value(QLatin1String("evidence")).toArray();
    if (evidences.isEmpty()) {
        return {};
    }

    const auto evidence = evidences.at(0).toObject();
    cert.setDate(QDateTime::fromString(evidence.value(QLatin1String("date")).toString(), Qt::ISODate).date());
    cert.setVaccine(evidence.value(QLatin1String("vaccine")).toString());
    cert.setManufacturer(evidence.value(QLatin1String("manufacturer")).toString());
    cert.setDose(evidence.value(QLatin1String("dose")).toInt());
    cert.setTotalDoses(evidence.value(QLatin1String("totalDoses")).toInt());

    const auto facility = evidence.value(QLatin1String("facility")).toObject();
    const auto address = facility.value(QLatin1String("address")).toObject();
    cert.setCountry(address.value(QLatin1String("addressCountry")).toString());

    cert.setCertificateId(evidence.value(QLatin1String("certificateId")).toString());
    cert.setCertificateIssuer(doc.object().value(QLatin1String("issuer")).toString());
    cert.setCertificateIssueDate(QDateTime::fromString(doc.object().value(QLatin1String("issuanceDate")).toString(), Qt::ISODate));
    cert.setSignatureState(KHealthCertificate::UncheckedSignature); // TODO implement signature verification

    return cert;
}

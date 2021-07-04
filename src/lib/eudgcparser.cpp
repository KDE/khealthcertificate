/*
 * SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "eudgcparser.h"
#include "cborutils_p.h"
#include "coseparser.h"

#include <KCodecs>

#include <QCborStreamReader>
#include <QDebug>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLocale>
#include <QVariant>

#include <zlib.h>

static QString translateValue(const QString &type, const QString &key)
{
    QFile f(QLatin1String(":/eu-dgc/") + type + QLatin1String(".json"));
    if (!f.open(QFile::ReadOnly)) {
        qWarning() << "no translation table found for" << type;
        return key;
    }

    const auto obj = QJsonDocument::fromJson(f.readAll()).object();
    const auto language = QLocale().name().left(QLocale().name().indexOf(QLatin1Char('_')));
    auto it = obj.constFind(key + QLatin1Char('[') + language + QLatin1Char(']'));
    if (it != obj.constEnd()) {
        return it.value().toString();
    }
    it = obj.constFind(key);
    if (it != obj.constEnd()) {
        return it.value().toString();
    }
    return key;
}

static QByteArray inflateByteArray(const QByteArray &data)
{
    // decompress
    QByteArray out;
    out.resize(4096);
    z_stream stream;
    stream.zalloc = nullptr;
    stream.zfree = nullptr;
    stream.opaque = nullptr;
    stream.avail_in = data.size();
    stream.next_in = reinterpret_cast<unsigned char*>(const_cast<char*>(data.data()));
    stream.avail_out = out.size();
    stream.next_out = reinterpret_cast<unsigned char*>(out.data());

    inflateInit(&stream);
    const auto res = inflate(&stream, Z_NO_FLUSH);
    switch (res) {
        case Z_OK:
        case Z_STREAM_END:
            break; // all good
        default:
            qWarning() << "zlib decompression failed" << stream.msg;
            return {};
    }
    inflateEnd(&stream);
    out.truncate(out.size() - stream.avail_out);
    return out;
}

QVariant EuDgcParser::parse(const QByteArray &data) const
{
    if (!data.startsWith("HC1:")) {
        return {};
    }

    const auto decoded = inflateByteArray(KCodecs::base45Decode(data.mid(4)));

    CoseParser cose;
    cose.parse(decoded);
    if (cose.payload.isEmpty()) {
        return {};
    }

    QCborStreamReader reader(cose.payload);
    if (!reader.isMap()) {
        return {};
    }
    reader.enterContainer();
    // parse certificate header
    QDateTime issueDt, expiryDt;
    while (reader.hasNext()) {
        const auto key = CborUtils::readInteger(reader);
        switch (key) {
            case -260:
                parseCertificate(reader);
                break;
            case 1:
                qDebug() << "key issuer:" << CborUtils::readString(reader);
                break;
            case 4:
                expiryDt = QDateTime::fromSecsSinceEpoch(CborUtils::readInteger(reader));
                break;
            case 6:
                issueDt = QDateTime::fromSecsSinceEpoch(CborUtils::readInteger(reader));
                break;
            default:
                qDebug() << "unhandled header key:" << key;
                reader.next();
        }
    }
    reader.leaveContainer();
    std::visit([&issueDt](auto &cert) { cert.setCertificateIssueDate(issueDt); }, m_cert);
    std::visit([&expiryDt](auto &cert) { cert.setCertificateExpiryDate(expiryDt); }, m_cert);
    std::visit([&data](auto &cert) { cert.setRawData(data); }, m_cert);
    return std::visit([](const auto &cert) { return QVariant::fromValue(cert); }, m_cert);
}

void EuDgcParser::parseCertificate(QCborStreamReader &reader) const
{
    if (!reader.isMap()) {
        return;
    }
    reader.enterContainer();
    const auto version = CborUtils::readInteger(reader);
    if (version != 1) {
        qDebug() << "unknown EU DGC version:" << version;
        return;
    }

    parseCertificateV1(reader);
}

void EuDgcParser::parseCertificateV1(QCborStreamReader &reader) const
{
    if (!reader.isMap()) {
        return;
    }
    reader.enterContainer();
    while (reader.hasNext()) {
        const auto key = CborUtils::readString(reader);
        if (key == QLatin1String("v")) {
            parseCertificateArray(reader, &EuDgcParser::parseVaccinationCertificate);
        } else if (key == QLatin1String("t")) {
            parseCertificateArray(reader, &EuDgcParser::parseTestCertificate);
        } else if (key == QLatin1String("r")) {
            parseCertificateArray(reader, &EuDgcParser::parseRecoveryCertificate);
        } else if (key == QLatin1String("nam")) {
            parseName(reader);
        } else if (key == QLatin1String("dob")) {
            const auto dob = QDate::fromString(CborUtils::readString(reader), Qt::ISODate);
            std::visit([&dob](auto &cert) { cert.setDateOfBirth(dob); }, m_cert);
        } else {
            qDebug() << "unhandled element:" << key;
            reader.next();
        }
    }
    reader.leaveContainer();
}

void EuDgcParser::parseCertificateArray(QCborStreamReader &reader, void (EuDgcParser::*func)(QCborStreamReader&) const) const
{
    if (!reader.isArray()) {
        return;
    }
    reader.enterContainer();
    while (reader.hasNext()) {
        (this->*func)(reader);
    }
    reader.leaveContainer();
}

void EuDgcParser::parseVaccinationCertificate(QCborStreamReader& reader) const
{
    if (!reader.isMap()) {
        return;
    }
    KVaccinationCertificate cert;
    reader.enterContainer();
    while (reader.hasNext()) {
        const auto key = CborUtils::readString(reader);
        if (key == QLatin1String("tg")) {
            cert.setDisease(translateValue(key, CborUtils::readString(reader)));
        } else if (key == QLatin1String("vp")) {
            cert.setVaccineType(translateValue(key, CborUtils::readString(reader)));
        } else if (key == QLatin1String("dt")) {
            cert.setDate(QDate::fromString(CborUtils::readString(reader), Qt::ISODate));
        } else if (key == QLatin1String("mp")) {
            cert.setVaccine(translateValue(key, CborUtils::readString(reader)));
        } else if (key == QLatin1String("ma")) {
            cert.setManufacturer(translateValue(key, CborUtils::readString(reader)));
        } else if (key == QLatin1String("dn")) {
            cert.setDose(CborUtils::readInteger(reader));
        } else if (key == QLatin1String("sd")) {
            cert.setTotalDoses(CborUtils::readInteger(reader));
        } else if (key == QLatin1String("co")) {
            cert.setCountry(CborUtils::readString(reader));
        } else if (key == QLatin1String("is")) {
            cert.setCertificateIssuer(CborUtils::readString(reader));
        } else if (key == QLatin1String("ci")) {
            cert.setCertificateId(CborUtils::readString(reader));
        } else {
            qDebug() << "unhandled vaccine key:" << key;
            reader.next();
        }
    }
    reader.leaveContainer();
    m_cert = std::move(cert);
}

void EuDgcParser::parseTestCertificate(QCborStreamReader &reader) const
{
    if (!reader.isMap()) {
        return;
    }
    KTestCertificate cert;
    reader.enterContainer();
    while (reader.hasNext()) {
        const auto key = CborUtils::readString(reader);
        if (key == QLatin1String("tg")) {
            cert.setDisease(translateValue(key, CborUtils::readString(reader)));
        } else if (key == QLatin1String("tt")) {
            cert.setTestType(translateValue(QLatin1String("tcTt"), CborUtils::readString(reader)));
        } else if (key == QLatin1String("nm")) {
            cert.setNaaTestName(CborUtils::readString(reader));
        } else if (key == QLatin1String("ma")) {
            cert.setRatTest(translateValue(QLatin1String("tcMa"), CborUtils::readString(reader)));
        } else if (key == QLatin1String("sc")) {
            cert.setDate(QDate::fromString(CborUtils::readString(reader), Qt::ISODate));
        } else if (key == QLatin1String("tr")) {
            const auto value = CborUtils::readString(reader);
            cert.setResultString(translateValue(QLatin1String("tcTr"), value));
            cert.setResult(value == QLatin1String("260415000") ? KTestCertificate::Negative : KTestCertificate::Positive);
        } else if (key == QLatin1String("tc")) {
            cert.setTextCenter(CborUtils::readString(reader));
        } else if (key == QLatin1String("co")) {
            cert.setCountry(CborUtils::readString(reader));
        } else if (key == QLatin1String("is")) {
            cert.setCertificateIssuer(CborUtils::readString(reader));
        } else if (key == QLatin1String("ci")) {
            cert.setCertificateId(CborUtils::readString(reader));
        } else {
            qDebug() << "unhandled test key:" << key;
            reader.next();
        }
    }
    reader.leaveContainer();
    m_cert = std::move(cert);
}

void EuDgcParser::parseRecoveryCertificate(QCborStreamReader &reader) const
{
    if (!reader.isMap()) {
        return;
    }
    KRecoveryCertificate cert;
    reader.enterContainer();
    while (reader.hasNext()) {
        const auto key = CborUtils::readString(reader);
       if (key == QLatin1String("tg")) {
            cert.setDisease(translateValue(key, CborUtils::readString(reader)));
        } else if (key == QLatin1String("fr")) {
            cert.setDateOfPositiveTest(QDate::fromString(CborUtils::readString(reader), Qt::ISODate));
        } else if (key == QLatin1String("df")) {
            cert.setValidFrom(QDate::fromString(CborUtils::readString(reader), Qt::ISODate));
        } else if (key == QLatin1String("du")) {
            cert.setValidUntil(QDate::fromString(CborUtils::readString(reader), Qt::ISODate));
        } else if (key == QLatin1String("is")) {
            cert.setCertificateIssuer(CborUtils::readString(reader));
        } else if (key == QLatin1String("ci")) {
            cert.setCertificateId(CborUtils::readString(reader));
        } else {
            qDebug() << "unhandled recovery key:" << key;
            reader.next();
        }
    }
    reader.leaveContainer();
    m_cert = std::move(cert);
}

void EuDgcParser::parseName(QCborStreamReader &reader) const
{
    if (!reader.isMap()) {
        return;
    }
    QString fn, gn;
    reader.enterContainer();
    while (reader.hasNext()) {
        const auto key = CborUtils::readString(reader);
        if (key == QLatin1String("fn")) {
            fn = CborUtils::readString(reader);
        } else if (key == QLatin1String("gn")) {
            gn = CborUtils::readString(reader);
        } else {
            reader.next();
        }
    }
    reader.leaveContainer();

    const QString name = gn + QLatin1Char(' ') + fn;
    std::visit([&name](auto &cert) { cert.setName(name); }, m_cert);
}

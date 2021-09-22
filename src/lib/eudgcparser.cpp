/*
 * SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "eudgcparser_p.h"
#include "cborutils_p.h"
#include "coseparser_p.h"
#include "logging.h"
#include "zlib_p.h"

#include <KCodecs>

#include <QCborStreamReader>
#include <QDebug>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLocale>
#include <QVariant>


static bool initResources()
{
    Q_INIT_RESOURCE(data);
    Q_INIT_RESOURCE(certs);
    return true;
}

EuDgcParser::EuDgcParser()
{
    [[maybe_unused]] static bool s_init = initResources();
}

EuDgcParser::~EuDgcParser() = default;

static QString translateValue(const QString &type, const QString &key)
{
    QFile f(QLatin1String(":/org.kde.khealthcertificate/eu-dgc/") + type + QLatin1String(".json"));
    if (!f.open(QFile::ReadOnly)) {
        qCWarning(Log) << "no translation table found for" << type;
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

QVariant EuDgcParser::parse(const QByteArray &data) const
{
    if (!data.startsWith("HC1:")) {
        return {};
    }

    const auto decoded = Zlib::decompressZlib(KCodecs::base45Decode(data.mid(4)));
    if (decoded.isEmpty()) {
        return {};
    }

    CoseParser cose;
    cose.parse(decoded);
    if (cose.payload().isEmpty()) {
        return {};
    }

    QCborStreamReader reader(cose.payload());
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
                qCDebug(Log) << "key issuer:" << CborUtils::readString(reader);
                break;
            case 4:
                expiryDt = QDateTime::fromSecsSinceEpoch(CborUtils::readInteger(reader));
                break;
            case 6:
                issueDt = QDateTime::fromSecsSinceEpoch(CborUtils::readInteger(reader));
                break;
            default:
                qCDebug(Log) << "unhandled header key:" << key;
                reader.next();
        }
    }
    reader.leaveContainer();
    std::visit([&issueDt](auto &cert) { cert.setCertificateIssueDate(issueDt); }, m_cert);
    std::visit([&expiryDt](auto &cert) { cert.setCertificateExpiryDate(expiryDt); }, m_cert);

    // signature validation
    auto sigState = cose.signatureState();
    if (sigState == CoseParser::ValidSignature && cose.certificate().expiryDate() < issueDt ) {
        sigState = CoseParser::InvalidSignature;
    }
    // TODO check key usage OIDs for 1.3.6.1.4.1.1847.2021.1.[1-3] / 1.3.6.1.4.1.0.1847.2021.1.[1-3]
    // (seems unused so far?)
    switch (sigState) {
        case CoseParser::InvalidSignature:
            std::visit([](auto &cert) { cert.setSignatureState(KHealthCertificate::InvalidSignature); }, m_cert);
            break;
        case CoseParser::ValidSignature:
            std::visit([](auto &cert) { cert.setSignatureState(KHealthCertificate::ValidSignature); }, m_cert);
            break;
        default:
            std::visit([](auto &cert) { cert.setSignatureState(KHealthCertificate::UnknownSignature); }, m_cert);
            break;
    }
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
        qCWarning(Log) << "unknown EU DGC version:" << version;
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
            qCDebug(Log) << "unhandled element:" << key;
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
            const auto productId = CborUtils::readString(reader);
            cert.setVaccine(translateValue(key,productId));
            if (productId.startsWith(QLatin1String("EU/")) && productId.count(QLatin1Char('/')) == 3) {
                const auto num = QStringView(productId).mid(productId.lastIndexOf(QLatin1Char('/')) + 1);
                cert.setVaccineUrl(QUrl(QLatin1String("https://ec.europa.eu/health/documents/community-register/html/h") + num + QLatin1String(".htm")));
            }
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
            qCDebug(Log) << "unhandled vaccine key:" << key;
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
            cert.setTestName(CborUtils::readString(reader));
        } else if (key == QLatin1String("ma")) {
            const auto productId = CborUtils::readString(reader);
            cert.setTestName(translateValue(QLatin1String("tcMa"), productId));
            cert.setTestUrl(QUrl(QLatin1String("https://covid-19-diagnostics.jrc.ec.europa.eu/devices/detail/") + productId));
        } else if (key == QLatin1String("sc")) {
            cert.setDate(QDate::fromString(CborUtils::readString(reader), Qt::ISODate));
        } else if (key == QLatin1String("tr")) {
            const auto value = CborUtils::readString(reader);
            cert.setResultString(translateValue(QLatin1String("tcTr"), value));
            cert.setResult(value == QLatin1String("260415000") ? KTestCertificate::Negative : KTestCertificate::Positive);
        } else if (key == QLatin1String("tc")) {
            cert.setTestCenter(CborUtils::readString(reader));
        } else if (key == QLatin1String("co")) {
            cert.setCountry(CborUtils::readString(reader));
        } else if (key == QLatin1String("is")) {
            cert.setCertificateIssuer(CborUtils::readString(reader));
        } else if (key == QLatin1String("ci")) {
            cert.setCertificateId(CborUtils::readString(reader));
        } else {
            qCDebug(Log) << "unhandled test key:" << key;
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
            qCDebug(Log) << "unhandled recovery key:" << key;
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

/*
 * SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "eudgcparser.h"
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

    // TODO parse certificate header
    qDebug() << reader.toInteger();
    reader.next();
    qDebug() << reader.readString().data;
    reader.next();
    qDebug() << reader.toInteger();
    reader.next();
    qDebug() << reader.toInteger();
    reader.next();
    qDebug() << reader.toInteger();
    reader.next();
    qDebug() << reader.toInteger();
    reader.next();

    // the actual certificate
    if (reader.toInteger() == -260) {
        reader.next();
        return parseCertificate(reader);
    }

    return {};
}

QVariant EuDgcParser::parseCertificate(QCborStreamReader &reader) const
{
    if (!reader.isMap()) {
        return {};
    }
    reader.enterContainer();
    const auto version = reader.toInteger();
    if (version != 1) {
        qDebug() << "unknown EU DGC version:" << version;
        return {};
    }

    reader.next();
    return parseCertificateV1(reader);
}

QVariant EuDgcParser::parseCertificateV1(QCborStreamReader &reader) const
{
    if (!reader.isMap()) {
        return {};
    }
    reader.enterContainer();
    while (reader.hasNext()) {
        const auto key = reader.readString().data;
        reader.next();
        if (key == QLatin1String("v")) {
            parseVaccinationCertificateArray(reader);
        } else if (key == QLatin1String("t")) {
            parseTestCertificateArray(reader);
        } else if (key == QLatin1String("r")) {
            parseRecoveryCertificateArray(reader);
        } else if (key == QLatin1String("nam")) {
            parseName(reader);
        } else if (key == QLatin1String("dob")) {
            const auto dob = QDate::fromString(reader.readString().data, Qt::ISODate);
            reader.next();
            std::visit([&dob](auto &cert) { cert.setDateOfBirth(dob); }, m_cert);
        } else {
            qDebug() << "unhandled element:" << key;
            reader.next();
        }
    }
    reader.leaveContainer();

    return std::visit([](const auto &cert) { return QVariant::fromValue(cert); }, m_cert);
}

void EuDgcParser::parseVaccinationCertificateArray(QCborStreamReader &reader) const
{
    if (!reader.isArray()) {
        return;
    }
    reader.enterContainer();
    // TODO can we have more than one here!?
    while (reader.hasNext()) {
        parseVaccinationCertificate(reader);
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
        const auto key = reader.readString().data;
        reader.next();
        if (key == QLatin1String("dt")) {
            cert.setDate(QDate::fromString(reader.readString().data, Qt::ISODate));
            reader.next();
        } else if (key == QLatin1String("mp")) {
            cert.setVaccine(translateValue(key, reader.readString().data));
            reader.next();
        } else if (key == QLatin1String("ma")) {
            cert.setManufacturer(translateValue(key, reader.readString().data));
            reader.next();
        } else if (key == QLatin1String("dn")) {
            cert.setDose(reader.toInteger());
            reader.next();
        } else if (key == QLatin1String("sd")) {
            cert.setTotalDoses(reader.toInteger());
            reader.next();
        } else {
            qDebug() << "unhandled vaccine key:" << key;
            reader.next();
        }
    }
    reader.leaveContainer();
    m_cert = std::move(cert);
}

void EuDgcParser::parseTestCertificateArray(QCborStreamReader &reader) const
{
    if (!reader.isArray()) {
        return;
    }
    reader.enterContainer();
    while (reader.hasNext()) {
        parseTestCertificate(reader);
    }
    reader.leaveContainer();}

void EuDgcParser::parseTestCertificate(QCborStreamReader &reader) const
{
    if (!reader.isMap()) {
        return;
    }
    KTestCertificate cert;
    reader.enterContainer();
    while (reader.hasNext()) {
        const auto key = reader.readString().data;
        reader.next();
        qDebug() << "unhandled test key:" << key;
        reader.next();
    }
    reader.leaveContainer();
    m_cert = std::move(cert);
}

void EuDgcParser::parseRecoveryCertificateArray(QCborStreamReader &reader) const
{
    if (!reader.isArray()) {
        return;
    }
    reader.enterContainer();
    while (reader.hasNext()) {
        parseTestCertificate(reader);
    }
    reader.leaveContainer();
}

void EuDgcParser::parseRecoveryCertificate(QCborStreamReader &reader) const
{
    if (!reader.isMap()) {
        return;
    }
    KRecoveryCertificate cert;
    reader.enterContainer();
    while (reader.hasNext()) {
        const auto key = reader.readString().data;
        reader.next();
        qDebug() << "unhandled recovery key:" << key;
        reader.next();
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
        const auto key = reader.readString().data;
        reader.next();
        if (key == QLatin1String("fn")) {
            fn = reader.readString().data;
            reader.next();
        } else if (key == QLatin1String("gn")) {
            gn = reader.readString().data;
            reader.next();
        } else {
            reader.next();
        }
    }
    reader.leaveContainer();

    const QString name = gn + QLatin1Char(' ') + fn;
    std::visit([&name](auto &cert) { cert.setName(name); }, m_cert);
}

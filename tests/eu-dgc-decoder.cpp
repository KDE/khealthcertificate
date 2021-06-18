/*
 * SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include <KCodecs>
#include <QCborStreamReader>
#include <QDebug>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

#include <zlib.h>

static QString cborDecodeString(QCborStreamReader &reader)
{
    QString result;
    auto r = reader.readString();
    while (r.status == QCborStreamReader::Ok) {
        result += r.data;
        r = reader.readString();
    }

    if (r.status == QCborStreamReader::Error) {
        // handle error condition
        result.clear();
    }
    return result;
}

static QJsonValue readOneValue(QCborStreamReader &reader)
{
    switch (reader.type()) {
        case QCborStreamReader::Map:
        {
            reader.enterContainer();
            QJsonObject obj;
            while (reader.hasNext()) {
                const auto key = readOneValue(reader);
                const auto value = readOneValue(reader);
                obj.insert(key.toVariant().toString(), value);
            }
            if (reader.lastError() == QCborError::NoError)
                reader.leaveContainer();
            return obj;
        }
        case QCborStreamReader::Array:
        {
            reader.enterContainer();
            QJsonArray array;
            while (reader.hasNext()) {
                const auto value = readOneValue(reader);
                array.push_back(value);
            }
            if (reader.lastError() == QCborError::NoError)
                reader.leaveContainer();
            return array;
        }
        case QCborStreamReader::String:
            return cborDecodeString(reader);
        case QCborStreamReader::UnsignedInteger:
        {
            const auto v = reader.toInteger();
            reader.next();
            return v;
        }
        case QCborStreamReader::NegativeInteger:
        {
            const auto v = -(qint64)reader.toNegativeInteger();
            reader.next();
            return v;
        }
    }
    return QStringLiteral("TODO");
}

int main(int argc, char **argv)
{
    QFile f;
    f.open(stdin, QFile::ReadOnly);
    const auto in = f.readAll();

    // strip prefix
    if (!in.startsWith("HC1:")) {
        return 1;
    }

    // base45 decode
    const auto base45Decoded = KCodecs::base45Decode(in.mid(4));

    // decompress
    QByteArray inflateOut;
    inflateOut.resize(4096);
    z_stream stream;
    stream.zalloc = nullptr;
    stream.zfree = nullptr;
    stream.opaque = nullptr;
    stream.avail_in = base45Decoded.size();
    stream.next_in = reinterpret_cast<unsigned char*>(const_cast<char*>(base45Decoded.data()));
    stream.avail_out = inflateOut.size();
    stream.next_out = reinterpret_cast<unsigned char*>(inflateOut.data());

    inflateInit(&stream);
    const auto res = inflate(&stream, Z_NO_FLUSH);
    switch (res) {
        case Z_OK:
        case Z_STREAM_END:
            break; // all good
        default:
            qWarning() << "zlib decompression failed" << stream.msg;
            return 1;
    }
    inflateEnd(&stream);
    inflateOut.truncate(inflateOut.size() - stream.avail_out);

    // unpack COSE
    QCborStreamReader cborReader(inflateOut);
    cborReader.next();
    assert(cborReader.isArray());
    cborReader.enterContainer();
    cborReader.next();
    cborReader.next();
    QByteArray cbor2 = cborReader.readByteArray().data;

    // decode CBOR payload
    QCborStreamReader cborReader2(cbor2);
    QJsonDocument doc(readOneValue(cborReader2).toObject());
    qDebug().noquote() << doc.toJson();

    return 0;
}

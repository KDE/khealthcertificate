/*
 * SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "zlib_p.h"
#include "logging.h"

#include <QByteArray>

#include <zlib.h>

static QByteArray decompress(const QByteArray &data, int windowBits)
{
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

    inflateInit2(&stream, windowBits);
    const auto res = inflate(&stream, Z_NO_FLUSH);
    switch (res) {
        case Z_OK:
        case Z_STREAM_END:
            break; // all good
        default:
            qCWarning(Log) << "zlib decompression failed" << stream.msg;
            return {};
    }
    inflateEnd(&stream);
    out.truncate(out.size() - stream.avail_out);
    return out;
}

QByteArray Zlib::decompressZlib(const QByteArray &data)
{
    return decompress(data, MAX_WBITS);
}

QByteArray Zlib::decompressDeflate(const QByteArray &data)
{
    return decompress(data, -MAX_WBITS);
}

/*
 * SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "khealthcertificateparser.h"
#include "divocparser_p.h"
#include "eudgcparser_p.h"

#include <KZip>

#include <QBuffer>
#include <QByteArray>
#include <QVariant>

QVariant KHealthCertificateParser::parse(const QByteArray &data)
{
    EuDgcParser eudcg;
    auto result = eudcg.parse(data);
    if (!result.isNull()) {
        return result;
    }
    result = DivocParser::parse(data);
    if (!result.isNull()) {
        return result;
    }

    // ZIP unpacking (needed for Indian certificates)
    if (data.startsWith(QByteArray("\x50\x4B\x03\x04"))) {
        QBuffer buffer;
        buffer.setData(data);
        buffer.open(QIODevice::ReadOnly);
        KZip zip(&buffer);
        if (!zip.open(QIODevice::ReadOnly)) {
            return {};
        }
        const auto entries = zip.directory()->entries();
        for (const auto &entry : entries) {
            if (auto f = zip.directory()->file(entry)) {
                result = parse(f->data());
                if (!result.isNull()) {
                    auto vac = result.value<KVaccinationCertificate>();
                    vac.setRawData(data);
                    return vac;
                }
            }
        }

    }

    return {};
}

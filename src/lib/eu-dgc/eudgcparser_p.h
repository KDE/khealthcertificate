/*
 * SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef EUDGCPARSER_P_H
#define EUDGCPARSER_P_H

#include "krecoverycertificate.h"
#include "ktestcertificate.h"
#include "kvaccinationcertificate.h"

#include <QString>

#include <variant>

class QByteArray;
class QCborStreamReader;
class QVariant;

/** Parser for EU DGC certificates. */
class EuDgcParser
{
public:
    EuDgcParser();
    ~EuDgcParser();
    QVariant parse(const QByteArray &data) const;

private:
    void parseCertificate(QCborStreamReader &reader) const;
    void parseCertificateV1(QCborStreamReader &reader) const;
    void parseCertificateArray(QCborStreamReader &reader, void (EuDgcParser::* func)(QCborStreamReader&) const) const;
    void parseVaccinationCertificate(QCborStreamReader &reader) const;
    void parseTestCertificate(QCborStreamReader &reader) const;
    void parseRecoveryCertificate(QCborStreamReader &reader) const;
    void parseName(QCborStreamReader &reader) const;

    mutable std::variant<KVaccinationCertificate, KTestCertificate, KRecoveryCertificate> m_cert;
};

#endif // EUDGCPARSER_P_H

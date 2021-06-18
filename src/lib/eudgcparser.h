/*
 * SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef EUDGCPARSER_H
#define EUDGCPARSER_H

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
    QVariant parse(const QByteArray &data) const;

private:
    QVariant parseCertificate(QCborStreamReader &reader) const;
    QVariant parseCertificateV1(QCborStreamReader &reader) const;
    void parseVaccinationCertificateArray(QCborStreamReader &reader) const;
    void parseVaccinationCertificate(QCborStreamReader &reader) const;
    void parseTestCertificateArray(QCborStreamReader &reader) const;
    void parseTestCertificate(QCborStreamReader &reader) const;
    void parseRecoveryCertificateArray(QCborStreamReader &reader) const;
    void parseRecoveryCertificate(QCborStreamReader &reader) const;
    void parseName(QCborStreamReader &reader) const;

    mutable std::variant<KVaccinationCertificate, KTestCertificate, KRecoveryCertificate> m_cert;
};

#endif // EUDGCPARSER_H

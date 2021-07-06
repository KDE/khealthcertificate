/*
 * SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "coseparser.h"
#include "cborutils_p.h"

#include <QCborMap>
#include <QCborStreamReader>
#include <QCborValue>
#include <QDebug>
#include <QFile>

#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/x509.h>

enum {
    CoseHeaderAlgorithm = 1,
    CoseHeaderKid = 4
};

void CoseParser::parse(const QByteArray &data)
{
    // only single signer case implemented atm
    QCborStreamReader reader(data);
    if (reader.type() != QCborStreamReader::Tag || reader.toTag() != QCborKnownTags::COSE_Sign1) {
        qDebug() << "wrong COSE tag:" << reader.toTag();
        return;
    }

    reader.next();
    if (!reader.isArray()) {
        return;
    }

    reader.enterContainer();
    m_protectedParams = CborUtils::readByteArray(reader);
    auto params = QCborValue::fromCbor(m_protectedParams);
    qDebug() << params.toMap().value(CoseHeaderAlgorithm);
    params = QCborValue::fromCbor(reader);
    m_kid = params.toMap().value(CoseHeaderKid).toByteArray();
    m_payload = CborUtils::readByteArray(reader);
    m_signature = CborUtils::readByteArray(reader);

    // load certificate
    QFile certFile(QLatin1String(":/eu-dgc/certs/") + QString::fromUtf8(m_kid.toHex()) + QLatin1String(".pem"));
    if (!certFile.open(QFile::ReadOnly)) {
        qWarning() << "unable to find certificate for key id:" << m_kid.toHex();
        return;
    }

    const auto certData = certFile.readAll();
    auto bio = BIO_new_mem_buf(certData.constData(), certData.size());

    X509 *cert = PEM_read_bio_X509(bio, nullptr, nullptr, nullptr);
    if (!cert) {
        qWarning() << "failed to read X509 certificate";
    }

    EVP_PKEY *pkey = X509_get_pubkey(cert);
    if (!pkey) {
        qWarning() << "failed to load public key";
    }

    qDebug() << "found certificate!" << pkey << EVP_PKEY_bits(pkey);

    EVP_PKEY_free(pkey);
    X509_free(cert);
    BIO_free_all(bio);
}

QByteArray CoseParser::payload() const
{
    return m_payload;
}

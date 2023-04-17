/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <QFile>
#include <QTest>

#include <KHealthCertificateParser>
#include <KVaccinationCertificate>

void initLocale()
{
    qputenv("LC_ALL", "en_US.utf-8");
    qputenv("TZ", "UTC");
}

Q_CONSTRUCTOR_FUNCTION(initLocale)

class ShcParserTest : public QObject
{
    Q_OBJECT
private:
    QByteArray readFile(QStringView fileName) const
    {
        QFile f(QLatin1String(SOURCE_DIR "/data/") + fileName);
        if (!f.open(QFile::ReadOnly)) {
            qCritical() << f.errorString() << f.fileName();
        }
        return f.readAll();
    }

private Q_SLOTS:
    void testVaccinationCertificate()
    {
        auto cert = KHealthCertificateParser::parse(readFile(u"shc/example-00-f-qr-code-numeric-value-0.txt"));
        QCOMPARE(cert.userType(), qMetaTypeId<KVaccinationCertificate>());
        auto vac = cert.value<KVaccinationCertificate>();
        QCOMPARE(vac.name(), QLatin1String("John B. Anyperson"));
        QCOMPARE(vac.country(), QString());
        QCOMPARE(vac.dose(), 2);
        QCOMPARE(vac.totalDoses(), 0);
        QCOMPARE(vac.date(), QDate(2021, 1, 29));
        QCOMPARE(vac.disease(), QLatin1String("COVID-19"));
        QCOMPARE(vac.vaccine(), QLatin1String("COVID-19, mRNA, LNP-S, PF, 100 mcg or 50 mcg dose"));
        QCOMPARE(vac.manufacturer(), QLatin1String("Moderna US, Inc."));
        QCOMPARE(vac.certificateIssuer(), QLatin1String("https://spec.smarthealth.cards/examples/issuer"));
        QCOMPARE(vac.certificateId(), QString());
        QCOMPARE(vac.certificateIssueDate(), QDateTime({2021, 9, 21}, {21, 52, 14}));
        QCOMPARE(vac.signatureState(), KHealthCertificate::ValidSignature);
        QCOMPARE(vac.validationState(), KHealthCertificate::Valid);
        QCOMPARE(vac.vaccinationState(), KVaccinationCertificate::Vaccinated);
        QCOMPARE(vac.rawData(), readFile(u"shc/example-00-f-qr-code-numeric-value-0.txt"));
    }
};

QTEST_APPLESS_MAIN(ShcParserTest)

#include "shcparsertest.moc"

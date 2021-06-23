/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <QFile>
#include <QTest>

#include <KHealthCertificate/KHealthCertificateParser>
#include <KHealthCertificate/KRecoveryCertificate>
#include <KHealthCertificate/KTestCertificate>
#include <KHealthCertificate/KVaccinationCertificate>

void initLocale()
{
    qputenv("LC_ALL", "en_US.utf-8");
}

Q_CONSTRUCTOR_FUNCTION(initLocale)

class EuDgcParserTest : public QObject
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
        auto cert = KHealthCertificateParser::parse(readFile(u"eu-dgc/full-vaccination.txt"));
        QCOMPARE(cert.userType(), qMetaTypeId<KVaccinationCertificate>());
        auto vac = cert.value<KVaccinationCertificate>();
        QCOMPARE(vac.name(), QLatin1String("Erika Mustermann"));
        QCOMPARE(vac.disease(), QLatin1String("COVID-19"));
        QCOMPARE(vac.dateOfBirth(), QDate(1964, 8, 12));
        QCOMPARE(vac.country(), QLatin1String("DE"));
        QCOMPARE(vac.dose(), 2);
        QCOMPARE(vac.totalDoses(), 2);
        QCOMPARE(vac.date(), QDate(2021, 5, 29));
        QCOMPARE(vac.vaccine(), QLatin1String("COVID-19 Vaccine Moderna"));
        QCOMPARE(vac.manufacturer(), QLatin1String("Moderna Biotech Spain S.L."));
        QCOMPARE(vac.validationState(), KHealthCertificate::Valid);

        cert = KHealthCertificateParser::parse(readFile(u"eu-dgc/partial-vaccination.txt"));
        QCOMPARE(cert.userType(), qMetaTypeId<KVaccinationCertificate>());
        vac = cert.value<KVaccinationCertificate>();
        QCOMPARE(vac.name(), QLatin1String("Erika Mustermann"));
        QCOMPARE(vac.disease(), QLatin1String("COVID-19"));
        QCOMPARE(vac.dateOfBirth(), QDate(1964, 8, 12));
        QCOMPARE(vac.country(), QLatin1String("DE"));
        QCOMPARE(vac.dose(), 1);
        QCOMPARE(vac.totalDoses(), 2);
        QCOMPARE(vac.date(), QDate(2021, 5, 29));
        QCOMPARE(vac.vaccine(), QLatin1String("COVID-19 Vaccine Moderna"));
        QCOMPARE(vac.manufacturer(), QLatin1String("Moderna Biotech Spain S.L."));
        QCOMPARE(vac.validationState(), KHealthCertificate::Partial);
    }

    void testTestCertificates()
    {
        auto cert = KHealthCertificateParser::parse(readFile(u"eu-dgc/negative-test.txt"));
        QCOMPARE(cert.userType(), qMetaTypeId<KTestCertificate>());
        auto test = cert.value<KTestCertificate>();
        QCOMPARE(test.name(), QLatin1String("Erika Mustermann"));
        QCOMPARE(test.disease(), QLatin1String("COVID-19"));
        QCOMPARE(test.dateOfBirth(), QDate(1964, 8, 12));
        QCOMPARE(test.country(), QLatin1String("DE"));
        QCOMPARE(test.date(), QDate(2021, 5, 30));
        QCOMPARE(test.testType(), QLatin1String("Rapid immunoassay"));
        QCOMPARE(test.result(), KTestCertificate::Negative);
        QCOMPARE(test.resultString(), QLatin1String("Not detected"));
        QCOMPARE(test.validationState(), KHealthCertificate::Invalid); // expired
    }

    void testRecoveryCertificates()
    {
        auto cert = KHealthCertificateParser::parse(readFile(u"eu-dgc/recovery.txt"));
        QCOMPARE(cert.userType(), qMetaTypeId<KRecoveryCertificate>());
        auto test = cert.value<KRecoveryCertificate>();
        QCOMPARE(test.name(), QLatin1String("Erika Mustermann"));
        QCOMPARE(test.disease(), QLatin1String("COVID-19"));
        QCOMPARE(test.dateOfBirth(), QDate(1964, 8, 12));
        QCOMPARE(test.dateOfPositiveTest(), QDate(2021, 1, 10));
        QCOMPARE(test.validFrom(), QDate(2021, 5, 29));
        QCOMPARE(test.validUntil(), QDate(2021, 6, 15));
        QCOMPARE(test.validationState(), KHealthCertificate::Unknown); // not implemented yet
    }
};

QTEST_APPLESS_MAIN(EuDgcParserTest)

#include "eudgcparsertest.moc"

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
    qputenv("TZ", "UTC");
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
        QCOMPARE(vac.vaccineType(), QLatin1String("SARS-CoV-2 mRNA vaccine"));
        QCOMPARE(vac.vaccine(), QLatin1String("Spikevax (previously COVID-19 Vaccine Moderna)"));
        QCOMPARE(vac.vaccineUrl().toString(), QLatin1String("https://ec.europa.eu/health/documents/community-register/html/h1507.htm"));
        QCOMPARE(vac.manufacturer(), QLatin1String("Moderna Biotech Spain S.L."));
        QCOMPARE(vac.certificateIssuer(), QLatin1String("Robert Koch-Institut"));
        QCOMPARE(vac.certificateId(), QLatin1String("URN:UVCI:01DE/IZ12345A/5CWLU12RNOB9RXSEOP6FG8#W"));
        QCOMPARE(vac.certificateIssueDate(), QDateTime({2021, 5, 29}, {19, 21, 13}));
        QCOMPARE(vac.certificateExpiryDate(), QDateTime({2022, 1, 28}, {7, 47, 53}));
        QCOMPARE(vac.signatureState(), KHealthCertificate::ValidSignature);
        QCOMPARE(vac.validationState(), KHealthCertificate::Valid);
        QCOMPARE(vac.vaccinationState(), KVaccinationCertificate::FullyVaccinated);
        QCOMPARE(vac.rawData(), readFile(u"eu-dgc/full-vaccination.txt"));
        QCOMPARE(KHealthCertificate::relevantUntil(vac), QDateTime({2022, 1, 28}, {7, 47, 53}));

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
        QCOMPARE(vac.vaccineType(), QLatin1String("SARS-CoV-2 mRNA vaccine"));
        QCOMPARE(vac.vaccine(), QLatin1String("Spikevax (previously COVID-19 Vaccine Moderna)"));
        QCOMPARE(vac.manufacturer(), QLatin1String("Moderna Biotech Spain S.L."));
        QCOMPARE(vac.certificateIssuer(), QLatin1String("Robert Koch-Institut"));
        QCOMPARE(vac.certificateId(), QLatin1String("URN:UVCI:01DE/IZ12345A/5CWLU12RNOB9RXSEOP6FG8#W"));
        QCOMPARE(vac.certificateIssueDate(), QDateTime({2021, 5, 29}, {19, 21, 13}));
        QCOMPARE(vac.certificateExpiryDate(), QDateTime({2022, 1, 28}, {7, 47, 53}));
        QCOMPARE(vac.signatureState(), KHealthCertificate::ValidSignature);
        QCOMPARE(vac.validationState(), KHealthCertificate::Partial);
        QCOMPARE(vac.vaccinationState(), KVaccinationCertificate::PartiallyVaccinated);
        QCOMPARE(vac.rawData(), readFile(u"eu-dgc/partial-vaccination.txt"));

        // Swiss certificates use another signature algorithm
        cert = KHealthCertificateParser::parse(readFile(u"eu-dgc/full-vaccination-ch.txt"));
        QCOMPARE(cert.userType(), qMetaTypeId<KVaccinationCertificate>());
        vac = cert.value<KVaccinationCertificate>();
        QCOMPARE(vac.name(), QLatin1String("Martina Studer"));
        QCOMPARE(vac.country(), QLatin1String("CH"));
        QCOMPARE(vac.signatureState(), KHealthCertificate::ValidSignature);
        QCOMPARE(vac.validationState(), KHealthCertificate::Valid);
        QCOMPARE(vac.vaccinationState(), KVaccinationCertificate::FullyVaccinated);
        QCOMPARE(vac.rawData(), readFile(u"eu-dgc/full-vaccination-ch.txt"));
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
        QCOMPARE(test.certificateIssuer(), QLatin1String("Robert Koch-Institut"));
        QCOMPARE(test.certificateId(), QLatin1String("URN:UVCI:01DE/IZ12345A/5CWLU12RNOB9RXSEOP6FG8#W"));
        QCOMPARE(test.certificateIssueDate(), QDateTime({2021, 5, 29}, {19, 21, 13}));
        QCOMPARE(test.certificateExpiryDate(), QDateTime({2022, 1, 28}, {7, 47, 53}));
        QCOMPARE(test.signatureState(), KHealthCertificate::ValidSignature);
        QCOMPARE(test.validationState(), KHealthCertificate::Partial); // expired
        QCOMPARE(test.isCurrent(), false);
        QCOMPARE(test.rawData(), readFile(u"eu-dgc/negative-test.txt"));
        QCOMPARE(KHealthCertificate::relevantUntil(test), QDateTime({2021, 6, 1}, {}));

        cert = KHealthCertificateParser::parse(readFile(u"eu-dgc/negative-rat-test-cz.txt"));
        QCOMPARE(cert.userType(), qMetaTypeId<KTestCertificate>());
        test = cert.value<KTestCertificate>();
        QCOMPARE(test.name(), QStringLiteral("Jan Novák"));
        QCOMPARE(test.disease(), QLatin1String("COVID-19"));
        QCOMPARE(test.country(), QLatin1String("CZ"));
        QCOMPARE(test.testType(), QLatin1String("Rapid immunoassay"));
        QVERIFY(!test.testName().isEmpty());
        QCOMPARE(test.testUrl().toString(), QLatin1String("https://covid-19-diagnostics.jrc.ec.europa.eu/devices/detail/1242"));
        QCOMPARE(test.result(), KTestCertificate::Negative);
        QCOMPARE(test.signatureState(), KHealthCertificate::ValidSignature);
        QCOMPARE(test.isCurrent(), false);
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
        QCOMPARE(test.certificateIssuer(), QLatin1String("Robert Koch-Institut"));
        QCOMPARE(test.certificateId(), QLatin1String("URN:UVCI:01DE/5CWLU12RNOB9RXSEOP6FG8#W"));
        QCOMPARE(test.certificateIssueDate(), QDateTime({2021, 5, 29}, {19, 21, 13}));
        QCOMPARE(test.certificateExpiryDate(), QDateTime({2022, 1, 28}, {7, 47, 53}));
        QCOMPARE(test.signatureState(), KHealthCertificate::ValidSignature);
        QCOMPARE(test.validationState(), KHealthCertificate::Unknown); // not implemented yet
        QCOMPARE(test.rawData(), readFile(u"eu-dgc/recovery.txt"));
        QCOMPARE(KHealthCertificate::relevantUntil(test), QDateTime({2021, 6, 15}, {}));
    }
};

QTEST_APPLESS_MAIN(EuDgcParserTest)

#include "eudgcparsertest.moc"

/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <QFile>
#include <QTest>

#include <KHealthCertificate/KHealthCertificateParser>
#include <KHealthCertificate/KVaccinationCertificate>
#include <KHealthCertificate/KTestCertificate>

void initLocale()
{
    qputenv("LC_ALL", "en_US.utf-8");
    qputenv("TZ", "UTC");
}

Q_CONSTRUCTOR_FUNCTION(initLocale)

class IcaoVdsParserTest : public QObject
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
        auto cert = KHealthCertificateParser::parse(readFile(u"icao/single-vaccine.txt"));
        QCOMPARE(cert.userType(), qMetaTypeId<KVaccinationCertificate>());
        auto vac = cert.value<KVaccinationCertificate>();
        QCOMPARE(vac.name(), QLatin1String("Smith Bill"));
        QCOMPARE(vac.dateOfBirth(), QDate(1990, 1, 2));
        QCOMPARE(vac.country(), QLatin1String("AUS"));
        QCOMPARE(vac.dose(), 1);
        QCOMPARE(vac.totalDoses(), 0);
        QCOMPARE(vac.date(), QDate(2021, 3, 3));
        QCOMPARE(vac.disease(), QLatin1String("COVID-19"));
        QCOMPARE(vac.vaccine(), QLatin1String("Comirnaty"));
        QCOMPARE(vac.vaccineType(), QLatin1String("COVID-19 vaccines"));
        QCOMPARE(vac.manufacturer(), QString());
        QCOMPARE(vac.certificateIssuer(), QString());
        QCOMPARE(vac.certificateId(), QLatin1String("U32870"));
        QCOMPARE(vac.certificateIssueDate(), QDateTime());
        QCOMPARE(vac.signatureState(), KHealthCertificate::UncheckedSignature);
        QCOMPARE(vac.validationState(), KHealthCertificate::Valid);
        QCOMPARE(vac.vaccinationState(), KVaccinationCertificate::Vaccinated);
        QCOMPARE(vac.rawData(), readFile(u"icao/single-vaccine.txt"));
    }

    void testTestCertificate()
    {
        auto cert = KHealthCertificateParser::parse(readFile(u"icao/test.txt"));
        QCOMPARE(cert.userType(), qMetaTypeId<KTestCertificate>());
        auto test = cert.value<KTestCertificate>();
        QCOMPARE(test.name(), QLatin1String("Cook Gerald"));
        QCOMPARE(test.dateOfBirth(), QDate(1990, 1, 29));
        QCOMPARE(test.country(), QLatin1String("AUS"));
        QCOMPARE(test.testCenter(), QLatin1String("General Hospital"));
        QCOMPARE(test.date(), QDate(2020, 12, 12));
        QCOMPARE(test.result(), KTestCertificate::Negative);
        QCOMPARE(test.resultString(), QLatin1String("negative"));
        QCOMPARE(test.testType(), QLatin1String("molecular(PCR)"));
        QCOMPARE(test.certificateIssuer(), QString());
        QCOMPARE(test.certificateId(), QLatin1String("U01932"));
        QCOMPARE(test.certificateIssueDate(), QDateTime());
        QCOMPARE(test.signatureState(), KHealthCertificate::UncheckedSignature);
        QCOMPARE(test.rawData(), readFile(u"icao/test.txt"));
    }
};

QTEST_APPLESS_MAIN(IcaoVdsParserTest)

#include "icaovdsparsertest.moc"

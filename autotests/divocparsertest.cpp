/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <QFile>
#include <QTest>

#include <KHealthCertificate/KHealthCertificateParser>
#include <KHealthCertificate/KVaccinationCertificate>

void initLocale()
{
    qputenv("LC_ALL", "en_US.utf-8");
    qputenv("TZ", "UTC");
}

Q_CONSTRUCTOR_FUNCTION(initLocale)

class DivocParserTest : public QObject
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
        auto cert = KHealthCertificateParser::parse(readFile(u"divoc/partial-vaccination.bin"));
        QCOMPARE(cert.userType(), qMetaTypeId<KVaccinationCertificate>());
        auto vac = cert.value<KVaccinationCertificate>();
        QCOMPARE(vac.name(), QLatin1String("Katie Dragon"));
        QCOMPARE(vac.country(), QLatin1String("IN"));
        QCOMPARE(vac.dose(), 1);
        QCOMPARE(vac.totalDoses(), 2);
        QCOMPARE(vac.date(), QDate(2021, 7, 16));
        QCOMPARE(vac.vaccine(), QLatin1String("COVISHIELD"));
        QCOMPARE(vac.certificateIssuer(), QLatin1String("https://cowin.gov.in/"));
        QCOMPARE(vac.certificateId(), QLatin1String("987654321098"));
        QCOMPARE(vac.certificateIssueDate(), QDateTime({2021, 7, 16}, {19, 40}));
        QCOMPARE(vac.signatureState(), KHealthCertificate::InvalidSignature);
        QCOMPARE(vac.validationState(), KHealthCertificate::Invalid); // due to invalid signature
        QCOMPARE(vac.vaccinationState(), KVaccinationCertificate::PartiallyVaccinated);
        QCOMPARE(vac.rawData(), readFile(u"divoc/partial-vaccination.bin"));
        QCOMPARE(KHealthCertificate::relevantUntil(vac), QDateTime({2022, 7, 16}, {0, 0}));
    }
};

QTEST_APPLESS_MAIN(DivocParserTest)

#include "divocparsertest.moc"

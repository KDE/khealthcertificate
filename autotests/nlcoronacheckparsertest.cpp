/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <QFile>
#include <QTest>
#include <QTimeZone>

#include <KHealthCertificate/KHealthCertificateParser>
#include <KHealthCertificate/KTestCertificate>

void initLocale()
{
    qputenv("LC_ALL", "en_US.utf-8");
    qputenv("TZ", "Europe/Amsterdam");
}

Q_CONSTRUCTOR_FUNCTION(initLocale)

class NlCoronaCheckParserTest : public QObject
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
    void testCertificate()
    {
        auto cert = KHealthCertificateParser::parse(readFile(u"nl-coronacheck/sample-one-day.txt"));
        QCOMPARE(cert.userType(), qMetaTypeId<KTestCertificate>());
        auto t = cert.value<KTestCertificate>();
        QCOMPARE(t.name(), QLatin1String("B B"));
        QCOMPARE(t.dateOfBirth(), QDate(1900, 7, 31));
        QCOMPARE(t.country(), QLatin1String("NL"));
        QCOMPARE(t.disease(), QLatin1String("COVID-19"));
        QCOMPARE(t.result(), KTestCertificate::Negative);
        QCOMPARE(t.certificateIssueDate(), QDateTime({2021, 7, 28}, {12, 0}, QTimeZone("Europe/Amsterdam")));
        QCOMPARE(t.certificateExpiryDate(), QDateTime({2021, 7, 29}, {13, 0}, QTimeZone("Europe/Amsterdam")));
        QCOMPARE(t.signatureState(), KHealthCertificate::InvalidSignature);
        QCOMPARE(t.validationState(), KHealthCertificate::Invalid);
        QCOMPARE(t.certificateId(), QLatin1String("6MkhqwdOJa40zJUr8RtM+Q=="));
        QCOMPARE(t.rawData(), readFile(u"nl-coronacheck/sample-one-day.txt"));

        QCOMPARE(KHealthCertificate::relevantUntil(t), QDateTime({2021, 7, 29}, {13, 0}, QTimeZone("Europe/Amsterdam")));
    }
};

QTEST_APPLESS_MAIN(NlCoronaCheckParserTest)

#include "nlcoronacheckparsertest.moc"

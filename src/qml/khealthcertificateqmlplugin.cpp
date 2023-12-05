/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <KHealthCertificate>
#include <KHealthCertificateParser>
#include <KRecoveryCertificate>
#include <KTestCertificate>
#include <KVaccinationCertificate>

#include <QCoreApplication>
#include <QQmlEngine>
#include <QQmlExtensionPlugin>

class KHealthCertificateQmlPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface")
public:
    void registerTypes(const char* uri) override;
};


class KHealthCertificateParserWrapper
{
    Q_GADGET
public:
    Q_INVOKABLE QVariant parse(const QVariant &data) const;
};

QVariant KHealthCertificateParserWrapper::parse(const QVariant &data) const
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    if (data.type() == QVariant::ByteArray) {
#else
    if (data.userType() == QMetaType::QByteArray) {
#endif
      return KHealthCertificateParser::parse(data.toByteArray());
    }
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    if (data.type() == QVariant::String) {
#else
    if (data.userType() == QMetaType::QString) {
#endif
      return KHealthCertificateParser::parse(data.toString().toUtf8());
    }
    return {};
}

void KHealthCertificateQmlPlugin::registerTypes(const char*)
{
    qRegisterMetaType<KRecoveryCertificate>();
    qRegisterMetaType<KTestCertificate>();
    qRegisterMetaType<KVaccinationCertificate>();

    qmlRegisterUncreatableMetaObject(KHealthCertificate::staticMetaObject, "org.kde.khealthcertificate", 1, 0, "HealthCertificate", {});
    qmlRegisterUncreatableMetaObject(KTestCertificate::staticMetaObject, "org.kde.khealthcertificate", 1, 0, "TestCertificate", {});
    qmlRegisterUncreatableMetaObject(KVaccinationCertificate::staticMetaObject, "org.kde.khealthcertificate", 1, 0, "VaccinationCertificate", {});

    // HACK qmlplugindump chokes on gadget singletons, to the point of breaking ecm_find_qmlmodule()
    if (QCoreApplication::applicationName() != QLatin1String("qmlplugindump")) {
        qmlRegisterSingletonType("org.kde.khealthcertificate", 1, 0, "HealthCertificateParser", [](QQmlEngine*, QJSEngine *engine) -> QJSValue {
            return engine->toScriptValue(KHealthCertificateParserWrapper());
        });
    }
}

#include "khealthcertificateqmlplugin.moc"

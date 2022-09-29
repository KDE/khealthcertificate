/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <KHealthCertificate/KHealthCertificate>
#include <KHealthCertificate/KHealthCertificateParser>
#include <KHealthCertificate/KRecoveryCertificate>
#include <KHealthCertificate/KTestCertificate>
#include <KHealthCertificate/KVaccinationCertificate>

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
    if (data.type() == QVariant::ByteArray) {
        return KHealthCertificateParser::parse(data.toByteArray());
    }
    if (data.type() == QVariant::String) {
        return KHealthCertificateParser::parse(data.toString().toUtf8());
    }
    return {};
}

void KHealthCertificateQmlPlugin::registerTypes(const char*)
{
    qRegisterMetaType<KRecoveryCertificate>();
    qRegisterMetaType<KTestCertificate>();
    qRegisterMetaType<KVaccinationCertificate>();

    qmlRegisterUncreatableType<KHealthCertificate>("org.kde.khealthcertificate", 1, 0, "HealthCertificate", {});
    qmlRegisterUncreatableType<KTestCertificate>("org.kde.khealthcertificate", 1, 0, "TestCertificate", {});
    qmlRegisterUncreatableType<KVaccinationCertificate>("org.kde.khealthcertificate", 1, 0, "VaccinationCertificate", {});

    // HACK qmlplugindump chokes on gadget singletons, to the point of breaking ecm_find_qmlmodule()
    if (QCoreApplication::applicationName() != QLatin1String("qmlplugindump")) {
        qmlRegisterSingletonType("org.kde.khealthcertificate", 1, 0, "HealthCertificateParser", [](QQmlEngine*, QJSEngine *engine) -> QJSValue {
            return engine->toScriptValue(KHealthCertificateParserWrapper());
        });
    }
}

#include "khealthcertificateqmlplugin.moc"

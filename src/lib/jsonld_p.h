/*
 * SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef JSONLD_H
#define JSONLD_H

#include <QHash>
#include <QString>

#include <functional>
#include <vector>

namespace Rdf {
class Quad;
class Term;
}

class QByteArray;
class QJsonObject;
class QJsonValue;
class QUrl;

using JsonLdDocumentLoader = std::function<QByteArray(const QString&)>;
using JsonLdCurieMap = QHash<QString, QString>;
class JsonLdProperty;

class JsonLdMetaType
{
public:
    void load(const QJsonObject &obj);
    void addProperty(JsonLdProperty &&property);

    QString name;
    QString qualifiedName;
    std::vector<JsonLdProperty> properties;
};

class JsonLdProperty {
public:
    static JsonLdProperty fromJson(const QString &name, const QJsonValue &value);

    QString name;
    QString qualifiedName;
    QString type;
    QString prefix;
    JsonLdMetaType metaType;
};

class JsonLdContext
{
public:
    void load(const QByteArray &contextData, const JsonLdDocumentLoader &loader);
    void load(const QJsonObject &context);
    void resolve();
    JsonLdMetaType metaType(const QString &type) const;

    std::vector<JsonLdMetaType> metaTypes;
    JsonLdCurieMap curieMap;
    std::vector<JsonLdProperty> globalProperties;
};


/** JSON-LD to RDF conversion.
 *  @note This is far from a complete implementation of the full spec, this barely
 *  covers enough for the needs of DIVOC JWS verification.
 */
class JsonLd
{
public:
    // we only support offline data, so a synchronous interface is fine
    void setDocumentLoader(const JsonLdDocumentLoader &loader);

    /** Convert JSON-LD object to RDF */
    std::vector<Rdf::Quad> toRdf(const QJsonObject &obj) const;

private:
    Rdf::Term toRdfRecursive(const JsonLdContext &context, const QJsonObject &obj, std::vector<Rdf::Quad> &quads) const;
    void toRdfRecursive(const JsonLdContext &context, const JsonLdMetaType &mt, const Rdf::Term &id, const QJsonObject &obj, std::vector<Rdf::Quad> &quads) const;
    Rdf::Term idForObject(const QJsonObject &obj) const;

    JsonLdDocumentLoader m_documentLoader;
    mutable int m_blankNodeCounter = 0;
};

#endif // JSONLD_H

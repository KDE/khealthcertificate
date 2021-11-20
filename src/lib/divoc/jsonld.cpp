/*
 * SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "jsonld_p.h"
#include "rdf_p.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

static bool readCurie(const QJsonObject::const_iterator it, JsonLdCurieMap &curieMap)
{
    const auto prefix = it.value().toString();
    if (!prefix.startsWith(QLatin1String("http"))) {
        return false;
    }
    constexpr const char allowdTrailingChars[] = ":/?#[]@";
    if (std::count(std::begin(allowdTrailingChars), std::end(allowdTrailingChars), prefix.back().toLatin1()) == 0) {
        return false;
    }
    curieMap.insert(it.key(), prefix);
    return true;
}

static void resolveCurie(JsonLdProperty &prop, const JsonLdCurieMap &curieMap);

static void resolveCurie(QString &str, const JsonLdCurieMap &curieMap)
{
    const auto idx = str.indexOf(QLatin1Char(':'));
    if (idx < 0) {
        return;
    }
    const auto prefix = QStringView(str).left(idx);
    const auto fullPrefix = curieMap.value(prefix.toString());
    if (fullPrefix.isEmpty()) {
        return;
    }
    str = fullPrefix + QStringView(str).mid(idx + 1);
}

static void resolveCurie(JsonLdMetaType &mt, const JsonLdCurieMap &curieMap)
{
    resolveCurie(mt.qualifiedName, curieMap);
    for (auto &prop : mt.properties) {
        resolveCurie(prop, curieMap);
    }
}

static void resolveCurie(JsonLdProperty &prop, const JsonLdCurieMap &curieMap)
{
    resolveCurie(prop.qualifiedName, curieMap);
    resolveCurie(prop.type, curieMap);
    resolveCurie(prop.metaType, curieMap);
    resolveCurie(prop.prefix, curieMap);
}

JsonLdProperty JsonLdProperty::fromJson(const QString &name, const QJsonValue &value)
{
    JsonLdProperty prop;
    if (name == QLatin1String("id")) {
        return prop;
    }
    prop.name = name;
    if (value.isString()) {
        prop.qualifiedName = value.toString();
    } else {
        const auto obj = value.toObject();
        prop.qualifiedName = obj.value(QLatin1String("@id")).toString();
        prop.type = obj.value(QLatin1String("@type")).toString();
        if (prop.type == QLatin1String("@vocab")) {
            prop.prefix = prop.qualifiedName.left(prop.qualifiedName.indexOf(QLatin1Char(':')) + 1);
        }
    }
    return prop;
}


void JsonLdMetaType::load(const QJsonObject &obj)
{
    qualifiedName = obj.value(QLatin1String("@id")).toString();
    const auto context = obj.value(QLatin1String("@context")).toObject();
    JsonLdCurieMap curieMap;
    for (auto it = context.begin(); it != context.end(); ++it) {
        if (it.value().isObject()) {
            const auto propObj = it.value().toObject();
            auto prop = JsonLdProperty::fromJson(it.key(), it.value());
            if (propObj.contains(QLatin1String("@context"))) {
                prop.metaType.load(propObj);
            }
            addProperty(std::move(prop));
        }
        else if (it.value().isString()) {
            if (readCurie(it, curieMap)) {
                continue;
            }
            addProperty(JsonLdProperty::fromJson(it.key(), it.value()));
        }
    }

    resolveCurie(*this, curieMap);
}

void JsonLdMetaType::addProperty(JsonLdProperty &&property)
{
    if (property.name.isEmpty()) {
        return;
    }

    auto it = std::lower_bound(properties.begin(), properties.end(), property, [](const auto &lhs, const auto &rhs) { return lhs.name < rhs.name; });
    if (it == properties.end() || (*it).name != property.name) {
        properties.insert(it, std::move(property));
    }
}


void JsonLdContext::load(const QByteArray &contextData, const JsonLdDocumentLoader &loader)
{
    const auto doc = QJsonDocument::fromJson(contextData);
    const auto context = doc.object().value(QLatin1String("@context"));
    if (context.isObject()) {
        load(context.toObject());
    } else if (context.isArray()) {
        for (const auto &c : context.toArray()) {
            if (c.isObject()) {
                load(c.toObject());
            } else if (c.isString()) {
                load(loader(c.toString()), loader);
            }
        }
    }
}

void JsonLdContext::load(const QJsonObject &context)
{
    for (auto it = context.begin(); it != context.end(); ++it) {
        if (it.value().isObject()) {
            const auto subObj = it.value().toObject();
            if (it.key().front().isUpper() || subObj.contains(QLatin1String("@context"))) {
                JsonLdMetaType metaType;
                metaType.name = it.key();
                metaType.load(subObj);
                metaTypes.push_back(std::move(metaType));
            } else {
                globalProperties.push_back(JsonLdProperty::fromJson(it.key(), it.value()));
            }
        }
        else if (it.value().isString()) {
            if (readCurie(it, curieMap)) {
                continue;
            }
            if (it.key().front().isUpper()) {
                JsonLdMetaType metaType;
                metaType.name = it.key();
                metaType.qualifiedName = it.value().toString();
                metaTypes.push_back(std::move(metaType));
            } else {
                globalProperties.push_back(JsonLdProperty::fromJson(it.key(), it.value()));
            }
        }
    }
}

void JsonLdContext::resolve()
{
    for (auto &mt : metaTypes) {
        for (auto prop : globalProperties) { // copy is intentional!
            mt.addProperty(std::move(prop));
        }
    }

    for (auto &mt : metaTypes) {
        resolveCurie(mt, curieMap);
    }

    std::sort(metaTypes.begin(), metaTypes.end(), [](const auto &lhs, const auto &rhs) {
        return lhs.name < rhs.name;
    });
}

JsonLdMetaType JsonLdContext::metaType(const QString &type) const
{
    const auto it = std::lower_bound(metaTypes.begin(), metaTypes.end(), type, [](const auto &lhs, const auto &rhs) {
        return lhs.name < rhs;
    });
    if (it != metaTypes.end() && (*it).name == type) {
        return *it;
    }
    return {};
}

void JsonLd::setDocumentLoader(const JsonLdDocumentLoader &loader)
{
    m_documentLoader = loader;
}

std::vector<Rdf::Quad> JsonLd::toRdf(const QJsonObject &obj) const
{
    std::vector<Rdf::Quad> quads;

    // determine context
    const auto contextVal = obj.value(QLatin1String("@context"));
    JsonLdContext context;
    if (contextVal.isArray()) {
        for (const auto &contextV : contextVal.toArray()) {
            context.load(m_documentLoader(contextV.toString()), m_documentLoader);
        }
    } else if (contextVal.isString()) {
        context.load(m_documentLoader(contextVal.toString()), m_documentLoader);
    }
    context.resolve();

    toRdfRecursive(context, obj, quads);
    return quads;
}

Rdf::Term JsonLd::toRdfRecursive(const JsonLdContext &context, const QJsonObject &obj, std::vector<Rdf::Quad> &quads) const
{
    const auto id = idForObject(obj);

    // find meta type for this object
    const auto typeVal = obj.value(QLatin1String("type"));
    if (typeVal.isArray()) {
        for (const auto &typeV : typeVal.toArray()) {
            toRdfRecursive(context, context.metaType(typeV.toString()), id, obj, quads);
        }
    } else if (typeVal.isString()) {
        toRdfRecursive(context, context.metaType(typeVal.toString()), id, obj, quads);
    }

    return id;
}

void JsonLd::toRdfRecursive(const JsonLdContext &context, const JsonLdMetaType &mt, const Rdf::Term &id, const QJsonObject &obj, std::vector<Rdf::Quad> &quads) const
{
    if (mt.name.isEmpty() && mt.properties.empty()) { // meta type not found
        return;
    }

    for (const auto &property : mt.properties) {
        const auto val = obj.value(property.name);
        if (val.isUndefined()) {
            continue;
        }

        const auto createQuad = [&](const QJsonValue &value) {
            Rdf::Quad quad;
            quad.subject = id;
            quad.predicate.value = property.qualifiedName;
            quad.predicate.type = Rdf::Term::IRI;
            if (value.isString()) {
                quad.object.value = value.toString();
                if (property.type == QLatin1String("@id")) {
                    quad.object.type = Rdf::Term::IRI;
                } else if (property.qualifiedName == QLatin1String("@type")) {
                    quad.predicate.value = QStringLiteral("http://www.w3.org/1999/02/22-rdf-syntax-ns#type");
                    quad.object.type = Rdf::Term::IRI;
                    if (!property.metaType.qualifiedName.isEmpty()) {
                        quad.object.value = property.metaType.qualifiedName;
                    } else if (!mt.qualifiedName.isEmpty()) {
                        quad.object.value = mt.qualifiedName;
                    }
                } else if (property.type == QLatin1String("@vocab")) {
                    quad.object.type = Rdf::Term::IRI;
                    quad.object.value = property.prefix + quad.object.value;
                } else {
                    quad.object.type = Rdf::Term::Literal;
                    quad.object.literalType = property.type;
                }
            } else if (value.isObject()) {
                if (property.metaType.properties.empty()) {
                    quad.object = toRdfRecursive(context, value.toObject(), quads);
                } else {
                    const auto subId = idForObject(value.toObject());
                    toRdfRecursive(context, property.metaType, subId, value.toObject(), quads);
                    quad.object = subId;
                }
            } else if (value.isDouble()) {
                quad.object.value = QString::number(value.toInt());
                quad.object.type = Rdf::Term::Literal;
                quad.object.literalType = QStringLiteral("http://www.w3.org/2001/XMLSchema#integer");
            }
            quads.push_back(std::move(quad));
        };
        if (val.isArray()) {
            if (property.name == QLatin1String("type")) {
                // we are already iterating over that one!
                createQuad(mt.name);
            } else {
                for (const auto &i : val.toArray()) {
                    createQuad(i);
                }
            }
        } else {
            createQuad(val);
        }
    }
}

Rdf::Term JsonLd::idForObject(const QJsonObject &obj) const
{
    Rdf::Term id;
    id.value = obj.value(QLatin1String("id")).toString();
    if (id.value.isEmpty()) {
        id.type = Rdf::Term::BlankNode;
        id.value = QString::number(m_blankNodeCounter++);
    } else {
        id.type = Rdf::Term::IRI;
    }
    return id;
}

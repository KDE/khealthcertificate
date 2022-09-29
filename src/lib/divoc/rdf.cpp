/*
 * SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "rdf_p.h"

#include <QBuffer>
#include <QCryptographicHash>
#include <QHash>
#include <QMap>
#include <QIODevice>

bool Rdf::Term::operator<(const Rdf::Term& other) const
{
    if (type == other.type) {
        if (value == other.value) {
            return literalType < other.literalType;
        }
        return value < other.value;
    }
    return type < other.type;
}

bool Rdf::Term::operator==(const Rdf::Term& other) const
{
    return type == other.type && value == other.value && literalType == other.literalType;
}

bool Rdf::Quad::operator<(const Rdf::Quad &other) const
{
    if (subject == other.subject) {
        if (predicate == other.predicate) {
            return object < other.object;
        }
        return predicate < other.predicate;
    }
    return subject < other.subject;
}

// see https://json-ld.github.io/rdf-dataset-canonicalization/spec/#hash-first-degree-quads
static QByteArray hashFirstDegreeQuads(const std::vector<Rdf::Quad> &quads, const QString &refBlankNode)
{
    const auto renameBlankNode = [&refBlankNode](Rdf::Term &term) {
        if (term.type == Rdf::Term::BlankNode) {
            if (term.value == refBlankNode) {
                term.value = QStringLiteral("a");
            } else {
                term.value = QStringLiteral("z");
            }
        }
    };

    std::vector<Rdf::Quad> toHash;
    for (auto quad : quads) {
        renameBlankNode(quad.subject);
        renameBlankNode(quad.predicate);
        renameBlankNode(quad.object);
        toHash.push_back(std::move(quad));
    }

    std::sort(toHash.begin(), toHash.end());
    return QCryptographicHash::hash(serialize(toHash), QCryptographicHash::Sha256).toHex();
}

QByteArray Rdf::serialize(const std::vector<Rdf::Quad>& quads)
{
    QByteArray out;
    QBuffer buffer(&out);
    buffer.open(QIODevice::WriteOnly);
    Rdf::serialize(&buffer, quads);
    buffer.close();
    return out;
}

void Rdf::normalize(std::vector<Rdf::Quad>& quads)
{
    // see https://json-ld.github.io/rdf-dataset-canonicalization/spec/#algorithm
    QHash<QString, std::vector<Rdf::Quad>> blankNodeToQuadMap;
    for (const auto &quad : quads) {
        // ignores predicates and the same blank nodes used multiple times in a quad, as that doesn't happen for us
        if (quad.subject.type == Rdf::Term::BlankNode) {
            blankNodeToQuadMap[quad.subject.value].push_back(quad);
        }
        if (quad.object.type == Rdf::Term::BlankNode) {
            blankNodeToQuadMap[quad.object.value].push_back(quad);
        }
    }

    QMap<QByteArray, QString> hashToBlankNodeMap;
    for (auto it = blankNodeToQuadMap.begin(); it != blankNodeToQuadMap.end(); ++it) {
        hashToBlankNodeMap.insert(hashFirstDegreeQuads(it.value(), it.key()), it.key());
    }

    int c14nIdCounter = 0;
    QHash<QString, QString> blankNodeC14nMap;
    for (auto it = hashToBlankNodeMap.begin(); it != hashToBlankNodeMap.end(); ++it) {
        blankNodeC14nMap.insert(it.value(), QLatin1String("c14n") + QString::number(c14nIdCounter++));
    }

    const auto translateBlankNode = [&blankNodeC14nMap](Rdf::Term &term) {
        if (term.type == Rdf::Term::BlankNode) {
            const auto it = blankNodeC14nMap.constFind(term.value);
            if (it != blankNodeC14nMap.constEnd()) {
                term.value = it.value();
            }
        }
    };
    for (auto &quad : quads) {
        translateBlankNode(quad.subject);
        translateBlankNode(quad.predicate);
        translateBlankNode(quad.object);
    }

    std::sort(quads.begin(), quads.end());
    quads.erase(std::unique(quads.begin(), quads.end(), [](const auto &lhs, const auto &rhs) {
        return lhs.subject == rhs.subject && lhs.predicate == rhs.predicate && lhs.object == rhs.object;
    }), quads.end());
}

void Rdf::serialize(QIODevice *out, const std::vector<Rdf::Quad> &quads)
{
    for (const auto &quad : quads) {
        serialize(out, quad);
    }
}

void Rdf::serialize(QIODevice *out, const Rdf::Quad &quad)
{
    serialize(out, quad.subject);
    out->write(" ");
    serialize(out, quad.predicate);
    out->write(" ");
    serialize(out, quad.object);
    out->write(" .\n");
}

void Rdf::serialize(QIODevice* out, const Rdf::Term &term)
{
    switch (term.type) {
        case Term::IRI:
            out->write("<");
            out->write(term.value.toUtf8());
            out->write(">");
            break;
        case Term::BlankNode:
            out->write("_:");
            out->write(term.value.toUtf8());
            break;
        case Term::Literal:
            out->write("\"");
            out->write(term.value.toUtf8());
            out->write("\"");
            if (!term.literalType.isEmpty()) {
                out->write("^^<");
                out->write(term.literalType.toUtf8());
                out->write(">");
            }
            break;
        case Term::Undefined:
            out->write(term.value.toUtf8());
            break;
    }
}

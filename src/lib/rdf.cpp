/*
 * SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "rdf_p.h"

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

void Rdf::normalize(std::vector<Rdf::Quad>& quads)
{
    // TODO
    std::sort(quads.begin(), quads.end(), [](const auto &lhs, const auto &rhs) {
        if (lhs.subject == rhs.subject) {
            if (lhs.predicate == rhs.predicate) {
                return lhs.object < rhs.object;
            }
            return lhs.predicate < rhs.predicate;
        }
        return lhs.subject < rhs.subject;
    });
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
            out->write("_:c14n");
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

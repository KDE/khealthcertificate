/*
 * SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef RDF_H
#define RDF_H

#include <QString>

class QIODevice;

/** Universal RDF Dataset Normalization Algorithm 2015 (URDNA2015)
  * @note This is far from a complete implementation of the full spec, this barely
  * covers enough for the needs of DIVOC JWS verification.
  */
namespace Rdf
{
class Term {
public:
    enum Type {
        Undefined,
        IRI,
        Literal,
        BlankNode,
    } type = Undefined;
    QString value;
    QString literalType;

    bool operator<(const Term &other) const;
    bool operator==(const Term &other) const;
};

class Quad {
public:
    Term subject;
    Term predicate;
    Term object;
    // ### graph - not relevant for us

    bool operator<(const Quad &other) const;
};

/** Apply the Universal RDF Dataset Normalization Algorithm 2015 (URDNA2015) to @p quads. */
void normalize(std::vector<Rdf::Quad> &quads);

/** Write list of RDF quads to @p out. */
QByteArray serialize(const std::vector<Rdf::Quad> &quads);
void serialize(QIODevice *out, const std::vector<Rdf::Quad> &quads);
void serialize(QIODevice *out, const Rdf::Quad &quad);
void serialize(QIODevice *out, const Rdf::Term &term);

}

#endif // RDF_H

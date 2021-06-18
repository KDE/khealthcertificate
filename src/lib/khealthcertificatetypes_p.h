/*
 * SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KHEALTHCERTIFICATETYPES_P_H
#define KHEALTHCERTIFICATETYPES_P_H

#include "khealthcertificatetypes.h"

#include <QSharedData>
#include <QVariant>

#define KHEALTHCERTIFICATE_MAKE_GADGET(Class) \
K ## Class ## Certificate::K ## Class ## Certificate() : d(new K ## Class ## Certificate ## Private) {} \
K ## Class ## Certificate::K ## Class ## Certificate(const K ## Class ## Certificate&) = default; \
K ## Class ## Certificate::K ## Class ## Certificate(K ## Class ## Certificate&&) noexcept = default; \
K ## Class ## Certificate::~K ## Class ## Certificate() = default; \
K ## Class ## Certificate& K ## Class ## Certificate::operator=(const K ## Class ## Certificate&) = default; \
K ## Class ## Certificate& K ## Class ## Certificate::operator=(K ## Class ## Certificate&&) noexcept = default; \
K ## Class ## Certificate::operator QVariant() const { return QVariant::fromValue(*this); }

#define KHEALTHCERTIFICATE_MAKE_PROPERTY(Class, Type, Getter, Setter) \
Type K ## Class ## Certificate::Getter() const { return d->Getter; } \
void K ## Class ## Certificate::Setter(KHealthCertificateInternal::parameter_type<Type>::type value) \
{ \
    d.detach(); \
    d->Getter = value; \
}

#endif // KHEALTHCERTIFICATETYPES_P_H



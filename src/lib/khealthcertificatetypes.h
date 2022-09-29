/*
 * SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KHEALTHCERTIFICATETYPES_H
#define KHEALTHCERTIFICATETYPES_H

#include "khealthcertificate.h"

#include <QSharedDataPointer>

#include <type_traits>

class QVariant;

namespace KHealthCertificateInternal {
template <typename T>
struct parameter_type
{
    using type = typename std::conditional<std::is_fundamental<T>::value || std::is_enum<T>::value, T, const T&>::type;
};
}

#define KHEALTHCERTIFICATE_GADGET(Class) \
    Q_GADGET \
    Q_PROPERTY(KHealthCertificate::CertificateType type READ type) \
public: \
    K ## Class ## Certificate(); \
    K ## Class ## Certificate(K ## Class ## Certificate &&) noexcept; \
    K ## Class ## Certificate(const K ## Class ##Certificate &); \
    ~K ## Class ## Certificate(); \
    K ## Class ## Certificate& operator=(K ## Class ## Certificate &&) noexcept; \
    K ## Class ## Certificate& operator=(const K ## Class ## Certificate &); \
    operator QVariant () const; \
private: \
    KHealthCertificate::CertificateType type() const; \
    friend class K ## Class ## CertificatePrivate; \
    QExplicitlySharedDataPointer<K ## Class ## CertificatePrivate> d;

#define KHEALTHCERTIFICATE_PROPERTY(Type, Getter, Setter) \
public: \
    Q_PROPERTY(Type Getter READ Getter CONSTANT) \
    Type Getter() const; \
    void Setter(KHealthCertificateInternal::parameter_type<Type>::type value); \

#endif // KHEALTHCERTIFICATETYPES_H


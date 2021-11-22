/*
 * SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "irmaverifier_p.h"
#include "irmapublickey_p.h"

#include <QCryptographicHash>
#include <QDebug>

#include <cstring>

IrmaProof::IrmaProof()
    : disclosureTime(0)
    , C(openssl::bn_ptr(nullptr, &BN_free))
    , A(openssl::bn_ptr(nullptr, &BN_free))
    , EResponse(openssl::bn_ptr(nullptr, &BN_free))
    , VResponse(openssl::bn_ptr(nullptr, &BN_free))
{
}


// see https://github.com/minvws/nl-covid19-coronacheck-idemix/blob/main/common/common.go#L132
// SHA-256 of the string representation of @p timestampe, cut of to a defined maximum length
static openssl::bn_ptr calculateTimeBasedChallenge(int64_t timestamp)
{
    auto h = QCryptographicHash::hash(QByteArray::number((qlonglong)timestamp), QCryptographicHash::Sha256);
    h.truncate(16);

    openssl::bn_ptr res(BN_new(), &BN_free);
    BN_bin2bn(reinterpret_cast<const uint8_t*>(h.constData()), h.size(), res.get());
    return res;
}

// SHA-256 on the binary data of the input number, returned as a number
static openssl::bn_ptr bignum_sha256(const openssl::bn_ptr &in)
{
    QByteArray s;
    s.resize(BN_num_bytes(in.get()));
    BN_bn2bin(in.get(), reinterpret_cast<uint8_t*>(s.data()));

    const auto h = QCryptographicHash::hash(s, QCryptographicHash::Sha256);
    openssl::bn_ptr res(BN_new(), &BN_free);
    BN_bin2bn(reinterpret_cast<const uint8_t*>(h.constData()), h.size(), res.get());
    return res;
}

// see https://github.com/privacybydesign/gabi/blob/master/proofs.go#L194
static openssl::bn_ptr reconstructZ(const IrmaProof &proof, const IrmaPublicKey &pubKey)
{
    openssl::bn_ctx_ptr bnCtx(BN_CTX_new(), &BN_CTX_free);

    openssl::bn_ptr numerator(BN_new(), &BN_free), tmp(BN_new(), &BN_free);
    BN_one(numerator.get());
    BN_lshift(tmp.get(), numerator.get(), 596); // "Le-1"
    std::swap(tmp, numerator);

    BN_mod_exp(tmp.get(), proof.A.get(), numerator.get(), pubKey.N.get(), bnCtx.get());
    std::swap(tmp, numerator);

    for (std::size_t i = 0; i < proof.ADisclosed.size(); ++i) {
        openssl::bn_ptr exp(nullptr, &BN_free);
        if (BN_num_bits(proof.ADisclosed[i].get()) > 256) { // "Lm"
            exp = bignum_sha256(proof.ADisclosed[i]);
        }

        BN_mod_exp(tmp.get(), pubKey.R[i+1].get(), exp ? exp.get() : proof.ADisclosed[i].get(), pubKey.N.get(), bnCtx.get());
        openssl::bn_ptr tmp2(BN_new(), &BN_free);
        BN_mul(tmp2.get(), numerator.get(), tmp.get(), bnCtx.get());
        std::swap(tmp2, numerator);
    }

    openssl::bn_ptr known(BN_new(), &BN_free);
    BN_mod_inverse(known.get(), numerator.get(), pubKey.N.get(), bnCtx.get());
    BN_mul(tmp.get(), pubKey.Z.get(), known.get(), bnCtx.get());
    std::swap(tmp, known);

    openssl::bn_ptr knownC(BN_new(), &BN_free);
    BN_mod_inverse(tmp.get(), known.get(), pubKey.N.get(), bnCtx.get());
    BN_mod_exp(knownC.get(), tmp.get(), proof.C.get(), pubKey.N.get(), bnCtx.get());

    openssl::bn_ptr Ae(BN_new(), &BN_free);
    BN_mod_exp(Ae.get(), proof.A.get(), proof.EResponse.get(), pubKey.N.get(), bnCtx.get());
    openssl::bn_ptr Sv(BN_new(), &BN_free);
    BN_mod_exp(Sv.get(), pubKey.S.get(), proof.VResponse.get(), pubKey.N.get(), bnCtx.get());

    openssl::bn_ptr Rs(BN_new(), &BN_free);
    BN_one(Rs.get());
    for (std::size_t i = 0; i < proof.AResponses.size(); ++i) {
        openssl::bn_ptr tmp2(BN_new(), &BN_free);
        BN_mod_exp(tmp2.get(), pubKey.R[i].get(), proof.AResponses[i].get(), pubKey.N.get(), bnCtx.get());
        BN_mul(tmp.get(), Rs.get(), tmp2.get(), bnCtx.get());
        std::swap(tmp, Rs);
    }

    openssl::bn_ptr Z(BN_new(), &BN_free);
    BN_mul(Z.get(), knownC.get(), Ae.get(), bnCtx.get());

    BN_mul(tmp.get(), Z.get(), Rs.get(), bnCtx.get());
    std::swap(tmp, Z);
    BN_mod_mul(tmp.get(), Z.get(), Sv.get(), pubKey.N.get(), bnCtx.get());
    std::swap(tmp, Z);
    return Z;
}

// encode a sequence of arbitrary size INTEGERs into an ASN.1 SEQUENCE
static QByteArray asn1EncodeSequence(const std::vector<const BIGNUM*> &numbers)
{
    QByteArray payloadBuffer;
    for (auto number : numbers) {
        openssl::asn1_integer_ptr num(BN_to_ASN1_INTEGER(number, nullptr), &ASN1_INTEGER_free);
        openssl::asn1_type_ptr obj(ASN1_TYPE_new(), &ASN1_TYPE_free);
        ASN1_TYPE_set(obj.get(),  V_ASN1_INTEGER, num.release());

        uint8_t *buffer = nullptr;
        const auto size = i2d_ASN1_TYPE(obj.get(), &buffer);
        payloadBuffer.append(reinterpret_cast<const char*>(buffer), size);
        free(buffer);
    }

    QByteArray result;
    result.resize(ASN1_object_size(1, payloadBuffer.size(), V_ASN1_SEQUENCE));
    auto resultIt = reinterpret_cast<uint8_t*>(result.data());
    ASN1_put_object(&resultIt, 1, payloadBuffer.size(), V_ASN1_SEQUENCE, 0);
    std::memcpy(resultIt, payloadBuffer.constData(), payloadBuffer.size());
    return result;
}

// see https://github.com/privacybydesign/gabi/blob/master/prooflist.go#L77
bool IrmaVerifier::verify(const IrmaProof &proof, const IrmaPublicKey &pubKey)
{
    openssl::bn_ptr context(BN_new(), &BN_free);
    BN_one(context.get());

    const auto timeBasedChallenge = calculateTimeBasedChallenge(proof.disclosureTime);
    const auto Z = reconstructZ(proof, pubKey);

    // create challenge: https://github.com/privacybydesign/gabi/blob/master/proofs.go#L27
    openssl::bn_ptr numElements(BN_new(), &BN_free);
    BN_set_word(numElements.get(), 4);

    const auto encoded = asn1EncodeSequence({numElements.get(), context.get(), proof.A.get(), Z.get(), timeBasedChallenge.get()});
    const auto challenge = QCryptographicHash::hash(encoded, QCryptographicHash::Sha256);

    QByteArray proofC;
    proofC.resize(BN_num_bytes(proof.C.get()));
    BN_bn2bin(proof.C.get(), reinterpret_cast<uint8_t*>(proofC.data()));
    return proofC == challenge;
}

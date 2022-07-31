# SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
# SPDX-License-Identifier: BSD-3-Clause

if (OPENSSL_VERSION VERSION_EQUAL "2.0.0")
    set(HAVE_LIBRESSL TRUE)
endif()
configure_file(openssl/config-openssl_p.h.in ${CMAKE_CURRENT_BINARY_DIR}/config-openssl_p.h)

add_library(KHealthCertificate
    khealthcertificate.cpp
    khealthcertificateparser.cpp
    krecoverycertificate.cpp
    ktestcertificate.cpp
    kvaccinationcertificate.cpp

    divoc/divocparser.cpp
    divoc/jsonld.cpp
    divoc/jwsverifier.cpp
    divoc/rdf.cpp
    divoc/data/divoc-data.qrc

    eu-dgc/cborutils.cpp
    eu-dgc/coseparser.cpp
    eu-dgc/eudgcparser.cpp
    eu-dgc/data/eu-dgc-data.qrc
    eu-dgc/certs/eu-dgc-certs.qrc

    icao/icaovdsparser.cpp
    icao/data/icaovds-data.qrc
    icao/certs/icao-csca-certs.qrc

    nl-coronacheck/nlcoronacheckparser.cpp
    nl-coronacheck/nlbase45.cpp
    nl-coronacheck/irmapublickey.cpp
    nl-coronacheck/irmaverifier.cpp
    nl-coronacheck/keys/nl-public-keys.qrc

    openssl/verify.cpp

    shc/jwkloader.cpp
    shc/jwtparser.cpp
    shc/shcparser.cpp
    shc/data/shc-data.qrc
    shc/certs/shc-certs.qrc
    shc/certs/shc-certs-manual.qrc

    zlib/zlib.cpp
)
set_target_properties(KHealthCertificate PROPERTIES
    VERSION ${KHEALTHCERTIFICATE_VERSION}
    SOVERSION ${KHEALTHCERTIFICATE_SOVERSION}
    EXPORT_NAME KHealthCertificate
)

generate_export_header(KHealthCertificate BASE_NAME KHealthCertificate)
ecm_qt_declare_logging_category(KHealthCertificate
    HEADER logging.h
    IDENTIFIER Log
    CATEGORY_NAME org.kde.khealthcertificate
    DESCRIPTION "KHealthCertificate"
    EXPORT KHealthCertificateLogging
)

target_link_libraries(KHealthCertificate PUBLIC
    Qt::Core
)
target_link_libraries(KHealthCertificate PRIVATE
    KF5::Archive
    KF5::Codecs
    KF5::I18nLocaleData
    Qt::Network
    OpenSSL::Crypto
    ZLIB::ZLIB
)

ecm_generate_headers(KHealthCertificate_FORWARDING_HEADERS
    HEADER_NAMES
        KHealthCertificate
        KHealthCertificateParser
        KRecoveryCertificate
        KTestCertificate
        KVaccinationCertificate
    PREFIX KHealthCertificate
    REQUIRED_HEADERS KHealthCertificate_HEADERS
)

install(TARGETS KHealthCertificate EXPORT KHealthCertificateTargets ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES
    ${KHealthCertificate_FORWARDING_HEADERS}
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/KHealthCertificate
)
install(FILES
    ${KHealthCertificate_HEADERS}
    khealthcertificatetypes.h
    ${CMAKE_CURRENT_BINARY_DIR}/khealthcertificate_export.h
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/khealthcertificate
)

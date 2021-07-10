# SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
# SPDX-License-Identifier: BSD-3-Clause

add_library(KHealthCertificate
    cborutils.cpp
    coseparser.cpp
    eudgcparser.cpp
    khealthcertificate.cpp
    khealthcertificateparser.cpp
    krecoverycertificate.cpp
    ktestcertificate.cpp
    kvaccinationcertificate.cpp
    verifiablecredentialparser.cpp

    data/data.qrc
    data/eu-dgc/certs/certs.qrc
)

generate_export_header(KHealthCertificate BASE_NAME KHealthCertificate)
ecm_qt_declare_logging_category(KHealthCertificate
    HEADER logging.h
    IDENTIFIER Log
    CATEGORY_NAME org.kde.khealthcertificate
    DESCRIPTION "KHealthCertificate"
    EXPORT KHealthCertificateLogging
)

set_target_properties(KHealthCertificate PROPERTIES
    VERSION ${KHEALTHCERTIFICATE_VERSION}
    SOVERSION ${KHEALTHCERTIFICATE_SOVERSION}
    EXPORT_NAME KHealthCertificate
)
target_link_libraries(KHealthCertificate PUBLIC
    Qt::Core
)
target_link_libraries(KHealthCertificate PRIVATE
    KF5::Archive
    KF5::Codecs
    Qt5::Network
    OpenSSL::SSL
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

install(TARGETS KHealthCertificate EXPORT KHealthCertificateTargets ${INSTALL_TARGETS_DEFAULT_ARGS})
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

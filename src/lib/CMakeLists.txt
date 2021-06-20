# SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
# SPDX-License-Identifier: BSD-3-Clause

add_library(KHealthCertificate
    coseparser.cpp
    eudgcparser.cpp
    khealthcertificate.cpp
    khealthcertificateparser.cpp
    krecoverycertificate.cpp
    ktestcertificate.cpp
    kvaccinationcertificate.cpp
    verifiablecredentialparser.cpp

    data/data.qrc
)

generate_export_header(KHealthCertificate BASE_NAME KHealthCertificate)
set_target_properties(KHealthCertificate PROPERTIES
    VERSION ${KHealthCertificate_VERSION}
    SOVERSION ${KHealthCertificate_SOVERSION}
    EXPORT_NAME KHealthCertificate
)
target_link_libraries(KHealthCertificate PUBLIC
    Qt5::Core
)
target_link_libraries(KHealthCertificate PRIVATE
    KF5::Archive
    KF5::Codecs
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

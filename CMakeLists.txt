# SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)

# KDE Application Version, managed by release script
set (RELEASE_SERVICE_VERSION_MAJOR "25")
set (RELEASE_SERVICE_VERSION_MINOR "03")
set (RELEASE_SERVICE_VERSION_MICRO "70")
set (RELEASE_SERVICE_VERSION "${RELEASE_SERVICE_VERSION_MAJOR}.${RELEASE_SERVICE_VERSION_MINOR}.${RELEASE_SERVICE_VERSION_MICRO}")
project(KHealthCertificate VERSION ${RELEASE_SERVICE_VERSION})

set(KF_MIN_VERSION 5.240)
find_package(ECM ${KF_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMAddTests)
include(ECMGenerateHeaders)
include(ECMQtDeclareLoggingCategory)
include(ECMSetupVersion)
include(FeatureSummary)
include(GenerateExportHeader)
include(ECMDeprecationSettings)

ecm_setup_version(PROJECT
    VARIABLE_PREFIX KHEALTHCERTIFICATE
    VERSION_HEADER khealthcertificate_version.h
    PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KHealthCertificateConfigVersion.cmake"
    SOVERSION 1
)

# build-time dependencies
find_package(Qt6 ${QT_MIN_VERSION} REQUIRED COMPONENTS Core Network Qml Test)
find_package(KF6 ${KF_MIN_VERSION} REQUIRED COMPONENTS Archive Codecs I18n)
find_package(OpenSSL REQUIRED COMPONENTS Crypto)
set_package_properties("OpenSSL" PROPERTIES TYPE REQUIRED PURPOSE "Needed for signature verification.")
find_package(ZLIB)
set_package_properties("ZLIB" PROPERTIES TYPE REQUIRED PURPOSE "Needed for decoding EU DGC data.")

ecm_set_disabled_deprecation_versions(QT 6.4
     KF 5.105.0
    )

add_subdirectory(src)
if (BUILD_TESTING)
    add_subdirectory(autotests)
    add_subdirectory(tests)
endif()

set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KHealthCertificate")
configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/KHealthCertificateConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/KHealthCertificateConfig.cmake"
    INSTALL_DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
)

install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/KHealthCertificateConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/KHealthCertificateConfigVersion.cmake"
    DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
    COMPONENT Devel
)
install(EXPORT KHealthCertificateTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE KHealthCertificateTargets.cmake)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/khealthcertificate_version.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/KHealthCertificate COMPONENT Devel)
ecm_qt_install_logging_categories(
    EXPORT KHealthCertificateLogging
    FILE org_kde_khealthcertificate.categories
    DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR}
)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)

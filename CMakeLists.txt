# SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)

# KDE Application Version, managed by release script
set (RELEASE_SERVICE_VERSION_MAJOR "21")
set (RELEASE_SERVICE_VERSION_MINOR "07")
set (RELEASE_SERVICE_VERSION_MICRO "70")
set (RELEASE_SERVICE_VERSION "${RELEASE_SERVICE_VERSION_MAJOR}.${RELEASE_SERVICE_VERSION_MINOR}.${RELEASE_SERVICE_VERSION_MICRO}")
project(KHealthCertificate VERSION ${RELEASE_SERVICE_VERSION})

set(CMAKE_CXX_STANDARD 17)

set(KF5_MIN_VERSION 5.84) # for KCodecs::base45Decode
find_package(ECM ${KF5_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDEFrameworkCompilerSettings NO_POLICY_SCOPE) # switch to KDECompilerSettings once we depend on ECM 5.85
include(ECMAddTests)
include(ECMGenerateHeaders)
include(ECMQtDeclareLoggingCategory)
include(ECMSetupVersion)
include(FeatureSummary)
include(GenerateExportHeader)

ecm_setup_version(PROJECT
    VARIABLE_PREFIX KHEALTHCERTIFICATE
    VERSION_HEADER khealthcertificate_version.h
    PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KHealthCertificateConfigVersion.cmake"
    SOVERSION 1
)

# build-time dependencies
set(QT_MIN_VERSION 5.15)
find_package(Qt5 ${QT_MIN_VERSION} REQUIRED COMPONENTS Core Network Qml Test)
find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS Archive Codecs)
find_package(OpenSSL)
set_package_properties("OpenSSL" PROPERTIES TYPE REQUIRED PURPOSE "Needed for signature verification.")
find_package(ZLIB)
set_package_properties("ZLIB" PROPERTIES TYPE REQUIRED PURPOSE "Needed for decoding EU DGC data.")

add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0x050f00)

add_subdirectory(src)
if (BUILD_TESTING)
    add_subdirectory(autotests)
    add_subdirectory(tests)
endif()

set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/KHealthCertificate")
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
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/khealthcertificate_version.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/khealthcertificate COMPONENT Devel)
ecm_qt_install_logging_categories(
    EXPORT KHealthCertificateLogging
    FILE org_kde_khealthcertificate.categories
    DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR}
)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)

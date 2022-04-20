# SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
# SPDX-License-Identifier: BSD-3-Clause

add_library(khealthcertificateqmlplugin khealthcertificateqmlplugin.cpp)
target_link_libraries(khealthcertificateqmlplugin
    Qt::Qml
    KHealthCertificate
)

# make examples work without installation
set_property(TARGET khealthcertificateqmlplugin PROPERTY LIBRARY_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/org/kde/khealthcertificate)
file(COPY qmldir DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/org/kde/khealthcertificate)

install(TARGETS khealthcertificateqmlplugin DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/khealthcertificate)
install(FILES qmldir ${qml_SRC} DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/khealthcertificate)

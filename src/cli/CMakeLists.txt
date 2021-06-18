# SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
# SPDX-License-Identifier: BSD-3-Clause

add_executable(health-certificate-decode health-certificate-decode.cpp)
target_link_libraries(health-certificate-decode PRIVATE KHealthCertificate)

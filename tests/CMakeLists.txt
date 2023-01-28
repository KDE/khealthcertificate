# SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
# SPDX-License-Identifier: BSD-3-Clause

add_executable(eu-dgc-decoder eu-dgc-decoder.cpp)
target_link_libraries(eu-dgc-decoder PRIVATE Qt::Core KF${KF_MAJOR_VERSION}::Codecs ZLIB::ZLIB)

add_executable(nl-decoder nl-decoder.cpp)
target_link_libraries(nl-decoder PRIVATE Qt::Core OpenSSL::SSL KHealthCertificate)

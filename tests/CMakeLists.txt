# SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
# SPDX-License-Identifier: BSD-3-Clause

add_executable(eu-dgc-decoder eu-dgc-decoder.cpp)
target_link_libraries(eu-dgc-decoder PRIVATE Qt5::Core KF5::Codecs ZLIB::ZLIB)

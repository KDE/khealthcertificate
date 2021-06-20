# SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
# SPDX-License-Identifier: BSD-3-Clause

add_custom_command(
    OUTPUT ${CMAKE_SOURCE_DIR}/src/lib/data/eu-dgc/ma.json
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/update-eu-dgc-data.py --output ${CMAKE_SOURCE_DIR}/src/lib/data/eu-dgc/
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Updating EU DCG translation tables"
)
add_custom_target(update-eu-dgc-data DEPENDS ${CMAKE_SOURCE_DIR}/src/lib/data/eu-dgc/ma.json)

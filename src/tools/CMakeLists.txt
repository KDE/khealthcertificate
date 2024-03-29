# SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
# SPDX-License-Identifier: BSD-3-Clause

add_custom_command(
    OUTPUT ${CMAKE_SOURCE_DIR}/src/lib/data/eu-dgc/dummy.json
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/update-eu-dgc-data.py --output ${CMAKE_SOURCE_DIR}/src/lib/eu-dgc/data/
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Updating EU DCG translation tables"
)
add_custom_target(update-eu-dgc-data DEPENDS ${CMAKE_SOURCE_DIR}/src/lib/data/eu-dgc/dummy.json)

add_custom_command(
    OUTPUT ${CMAKE_SOURCE_DIR}/src/lib/data/eu-dgc/certs/dummy.pem
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/update-eu-dgc-certs.py --output ${CMAKE_SOURCE_DIR}/src/lib/eu-dgc/certs/
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Updating EU DCG certificates"
)
add_custom_target(update-eu-dgc-certs DEPENDS ${CMAKE_SOURCE_DIR}/src/lib/data/eu-dgc/certs/dummy.pem)

add_custom_command(
    OUTPUT ${CMAKE_SOURCE_DIR}/src/lib/shc/data/hl7-cvx-codes.dummy
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/update-hl7-cvx-data.py --output ${CMAKE_SOURCE_DIR}/src/lib/shc/data/hl7-cvx-codes.json
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Updating HL7 CVX codes"
)
add_custom_target(update-hl7-cvx DEPENDS ${CMAKE_SOURCE_DIR}/src/lib/shc/data/hl7-cvx-codes.dummy)

add_custom_command(
    OUTPUT ${CMAKE_SOURCE_DIR}/src/lib/data/shc/certs/dummy.jwk
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/update-shc-certs.py --output ${CMAKE_SOURCE_DIR}/src/lib/shc/certs/
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Updating SHC certificates"
)
add_custom_target(update-shc-certs DEPENDS ${CMAKE_SOURCE_DIR}/src/lib/data/shc/certs/dummy.jwk)

add_custom_command(
    OUTPUT ${CMAKE_SOURCE_DIR}/src/lib/icao/data/dummy.json
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/update-icd-11-data.py --output ${CMAKE_SOURCE_DIR}/src/lib/icao/data
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Updating WHO ICD-11 data"
)
add_custom_target(update-icd-11-data DEPENDS ${CMAKE_SOURCE_DIR}/src/lib/icao/data/dummy.json)

add_custom_command(
    OUTPUT ${CMAKE_SOURCE_DIR}/src/lib/data/nl-coronacheck/keys/dummy.xml
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/update-nl-public-keys.py --output ${CMAKE_SOURCE_DIR}/src/lib/nl-coronacheck/keys
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Updating NL CoronaCheck public keys"
)
add_custom_target(update-nl-public-keys DEPENDS ${CMAKE_SOURCE_DIR}/src/lib/data/nl-coronacheck/keys/dummy.xml)

add_custom_target(update-all DEPENDS
    update-eu-dgc-data
    update-eu-dgc-certs
    update-hl7-cvx
    update-shc-certs
    update-icd-11-data
    update-nl-public-keys
)

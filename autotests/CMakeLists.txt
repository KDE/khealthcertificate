# SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
# SPDX-License-Identifier: BSD-3-Clause

add_definitions(-DSOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")

ecm_add_test(eudgcparsertest.cpp data/eu-dgc/certs.qrc TEST_NAME eudgcparsertest LINK_LIBRARIES Qt::Test KHealthCertificate)

#!/usr/bin/env python3
# SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
# SPDX-License-Identifier: LGPL-2.0-or-later

import argparse
import base64
import json
import os
import requests

parser = argparse.ArgumentParser(description='Download public keys for validating NL domestic CoronaCheck')
parser.add_argument('--output', type=str, required=True, help='Path to which the output should be written to')
arguments = parser.parse_args()

os.makedirs(arguments.output, exist_ok = True)

publicKeyUrls = [
    'https://raw.githubusercontent.com/minvws/nl-covid19-coronacheck-mobile-core/main/testdata/public_keys.json'
]
excludedKeyIds = [
    'testPk',
    'TST-KEY-01',
    'VWS-TEST-0'
]

publicKeyFileNames = []
for publicKeyUrl in publicKeyUrls:
    req = requests.get(publicKeyUrl)
    pks = json.loads(req.text)
    for keyId,key in pks['nl_keys'].items():
        if keyId in excludedKeyIds:
            continue

        pkFileName = keyId + '.xml'
        pkPath = os.path.join(arguments.output, pkFileName)
        pkFile = open(pkPath, 'wb')
        pkFile.write(base64.b64decode(key['public_key']))
        pkFile.close()
        publicKeyFileNames.append(pkFileName)

qrcFile = open(os.path.join(arguments.output, 'nl-public-keys.qrc'), 'w')
qrcFile.write("""<!--
    SPDX-FileCopyrightText: none
    SPDX-License-Identifier: CC0-1.0
-->
<RCC>
  <qresource prefix="/org.kde.khealthcertificate/nl-coronacheck/keys">
""")
for fileName in publicKeyFileNames:
    qrcFile.write(f"    <file>{fileName}</file>\n")
qrcFile.write("""  </qresource>
</RCC>""")
qrcFile.close()

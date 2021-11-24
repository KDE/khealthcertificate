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

publicKeyUrl = 'https://verifier-api.coronacheck.nl/v6/verifier/public_keys'

publicKeyFileNames = []
req = requests.get(publicKeyUrl)
# TODO verify signature
envelope = json.loads(req.text)
pks = json.loads(base64.b64decode(envelope['payload']))
for key in pks['cl_keys']:
    pkFileName = key['id'] + '.xml'
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

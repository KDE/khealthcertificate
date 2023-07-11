#!/usr/bin/env python3
# SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
# SPDX-License-Identifier: LGPL-2.0-or-later

import argparse
import json
import os
import requests

parser = argparse.ArgumentParser(description='Download certificates for validating SHCs')
parser.add_argument('--output', type=str, required=True, help='Path to which the output should be written to')
arguments = parser.parse_args()

os.makedirs(arguments.output, exist_ok = True)

issuerUrls = [
    'https://covid19.quebec.ca/PreuveVaccinaleApi/issuer',
    'https://smarthealthcard.phsa.ca/v1/issuer',
    'https://myvaccinerecord.cdph.ca.gov/creds/'
]

jwkFileNames = []
for issuer in issuerUrls:
    req = requests.get(issuer + '/.well-known/jwks.json')
    if req.status_code == 404:
        continue
    jwks = json.loads(req.text)
    for key in jwks['keys']:
        jwkFileName = key['kid'] + '.jwk'
        jwkPath = os.path.join(arguments.output, jwkFileName)
        jwkFile = open(jwkPath, 'w')
        jwkFile.write(json.dumps(key))
        jwkFile.close()
        jwkFileNames.append(jwkFileName)

qrcFile = open(os.path.join(arguments.output, 'shc-certs.qrc'), 'w')
qrcFile.write("""<!--
    SPDX-FileCopyrightText: none
    SPDX-License-Identifier: CC0-1.0
-->
<RCC>
  <qresource prefix="/org.kde.khealthcertificate/shc/certs">
""")
for jwkFileName in jwkFileNames:
    qrcFile.write(f"    <file>{jwkFileName}</file>\n")
qrcFile.write("""  </qresource>
</RCC>""")
qrcFile.close()

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

vciDirectoryUrl = 'https://raw.githubusercontent.com/the-commons-project/vci-directory/main/vci-issuers.json'
vciDirectoryReq = requests.get(vciDirectoryUrl)
vciDirectory = json.loads(vciDirectoryReq.text)

issuerUrls = []
for issuer in vciDirectory['participating_issuers']:
    issuerUrls.append(issuer['iss'])

jwkFileNames = []
for issuer in issuerUrls:
    print(f"Downloading {issuer}...")
    try:
        req = requests.get(issuer + '/.well-known/jwks.json', timeout=10)
    except Exception as ex:
        print(f"    exception: {ex}")
        continue
    if req.status_code != 200:
        print(f"   failed: {req.status_code}")
        continue
    try:
        jwks = json.loads(req.text)
    except Exception as ex:
        print(f"    exception: {ex} - {req.text}")
        continue
    if not jwks or 'keys' not in jwks:
        print(f"    invalid JWKS:" << req.text)
        continue
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

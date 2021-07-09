#!/usr/bin/env python3
# SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
# SPDX-License-Identifier: LGPL-2.0-or-later

import argparse
import base64
import json
import os
import requests

parser = argparse.ArgumentParser(description='Download certificates for validating EU DGCs')
parser.add_argument('--output', type=str, required=True, help='Path to which the output should be written to')
arguments = parser.parse_args()

os.makedirs(arguments.output, exist_ok = True)

req = requests.get('https://de.dscg.ubirch.com/trustList/DSC/')
# TODO figure out how to validate the signature at the start of this data
jsonStart = str(req.content).index('{')
certs = json.loads(str(req.content)[jsonStart:-1])

# write out qrc file
pemFileNames = []
for cert in certs['certificates']:
    pemFileName = base64.b64decode(cert['kid']).hex() + ".pem"
    pemPath = os.path.join(arguments.output, pemFileName)
    pemFile = open(pemPath, 'w')
    pemFile.write('-----BEGIN CERTIFICATE-----\n')
    pemFile.write(cert['rawData'])
    pemFile.write('\n-----END CERTIFICATE-----')
    pemFile.close()
    pemFileNames.append(pemFileName)

pemFileNames.sort()
qrcFile = open(os.path.join(arguments.output, 'certs.qrc'), 'w')
qrcFile.write("""<!--
    SPDX-FileCopyrightText: none
    SPDX-License-Identifier: CC0-1.0
-->
<RCC>
  <qresource prefix="/org.kde.khealthcertificate/eu-dgc/certs">
""")
for pemFileName in pemFileNames:
    qrcFile.write(f"    <file>{pemFileName}</file>\n")
qrcFile.write("""  </qresource>
</RCC>""")
qrcFile.close()

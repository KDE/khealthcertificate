#!/usr/bin/env python3
# SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
# SPDX-License-Identifier: LGPL-2.0-or-later

import argparse
import base64
import json
import os
import requests
import subprocess

def runOpenSsl(args, data = None):
    proc = subprocess.Popen(f"openssl {args}", shell=True, stdout=subprocess.PIPE, stdin=subprocess.PIPE)
    if data != None:
        proc.stdin.write(data)
        proc.stdin.close()
    return proc.stdout.read()


parser = argparse.ArgumentParser(description='Download certificates for validating EU DGCs')
parser.add_argument('--output', type=str, required=True, help='Path to which the output should be written to')
arguments = parser.parse_args()

os.makedirs(arguments.output, exist_ok = True)

req = requests.get('https://de.dscg.ubirch.com/trustList/DSC/')
# TODO figure out how to validate the signature at the start of this data
jsonStart = str(req.content).index('{')
certs = json.loads(str(req.content)[jsonStart:-1])

# remove all existing certs so we clean up revoked/expired ones
for certFile in os.listdir(arguments.output):
    if certFile.endswith(".pem") or certFile.endswith('.der'):
        os.remove(os.path.join(arguments.output, certFile))

derFileNames = []
for cert in certs['certificates']:
    pemData = f"-----BEGIN CERTIFICATE-----\n{cert['rawData']}\n-----END CERTIFICATE-----"

    derFileName = base64.b64decode(cert['kid']).hex() + ".der"
    derPath = os.path.join(arguments.output, derFileName)
    runOpenSsl(f"x509 -outform der -out {derPath}", pemData.encode('utf-8'))
    derFileNames.append(derFileName)

derFileNames.sort()

# write out qrc file
qrcFile = open(os.path.join(arguments.output, 'eu-dgc-certs.qrc'), 'w')
qrcFile.write("""<!--
    SPDX-FileCopyrightText: none
    SPDX-License-Identifier: CC0-1.0
-->
<RCC>
  <qresource prefix="/org.kde.khealthcertificate/eu-dgc/certs">
""")
for derFileName in derFileNames:
    qrcFile.write(f"    <file>{derFileName}</file>\n")
qrcFile.write("""  </qresource>
</RCC>""")
qrcFile.close()

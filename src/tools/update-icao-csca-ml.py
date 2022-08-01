#!/usr/bin/env python3
# SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
# SPDX-License-Identifier: LGPL-2.0-or-later

import argparse
import os
import subprocess
import pyasn1
import shutil

from pyasn1.codec.der.decoder import decode
from pyasn1.codec.der.encoder import encode

derFileNames = []

def writeToFile(fileName, content):
    f = open(fileName, 'wb')
    f.write(content)
    f.close();

def runOpenSsl(args, data = None):
    proc = subprocess.Popen(f"openssl {args}", shell=True, stdout=subprocess.PIPE, stdin=subprocess.PIPE)
    if data != None:
        proc.stdin.write(data)
        proc.stdin.close()
    return proc.stdout.read()

def removeExistingCerts():
    for certFile in os.listdir(arguments.output):
        path = os.path.join(arguments.output, certFile)
        if certFile.endswith('.der'):
            os.remove(path)
        elif os.path.isdir(path):
            shutil.rmtree(path)

def unpackMlFile(mlFileName):
    print (f"processing {mlFileName}")
    mlPemFile = runOpenSsl(f"cms -inform der -noverify -verify -in {mlFileName}")
    pems, _ = pyasn1.codec.der.decoder.decode(mlPemFile)
    keyData = {}
    i = 0
    while True:
        pemObj = pems.getComponentByPosition(1).getComponentByPosition(i, instantiate=False, default=None)
        if pemObj == None:
            break;
        pemData = pyasn1.codec.der.encoder.encode(pemObj)
        keyId = runOpenSsl('x509 -inform der -text -noout | grep -A1 "Subject Key Identifier" | tail -n 1', pemData).strip().decode().replace(':', '').lower()
        serial = runOpenSsl('x509 -inform der -serial -noout', pemData).strip().decode().replace('serial=', '').lower()
        if not keyId in keyData:
            keyData[keyId] = []
        keyData[keyId].append((pemData, serial))
        i += 1

    # write certificates
    for keyId in keyData:
        if len(keyData[keyId]) == 1:
            writeToFile(os.path.join(arguments.output, f"{keyId}.der"), keyData[keyId][0][0])
            derFileNames.append(f"{keyId}.der")
        else:
            os.mkdir(os.path.join(arguments.output, keyId))
            for (cert,serial) in keyData[keyId]:
                writeToFile(os.path.join(arguments.output, keyId, f"{serial}.der"), cert)
                derFileNames.append(f"{keyId}/{serial}.der")

def writeQrcFile():
    derFileNames.sort()
    qrcFile = open(os.path.join(arguments.output, 'icao-csca-certs.qrc'), 'w')
    qrcFile.write("""<!--
    SPDX-FileCopyrightText: none
    SPDX-License-Identifier: CC0-1.0
-->
<RCC>
  <qresource prefix="/org.kde.khealthcertificate/icao/certs">
""")
    for derFileName in derFileNames:
        qrcFile.write(f"    <file>{derFileName}</file>\n")
    qrcFile.write("  </qresource>\n</RCC>")
    qrcFile.close()


parser = argparse.ArgumentParser(description='Download and unpack ICAO CSCA master lists.')
parser.add_argument('--output', type=str, required=True, help='Path to which the output should be written to')
arguments = parser.parse_args()

removeExistingCerts()

#unpackMlFile('ICAO_health_ml_27May2022.ml')
#unpackMlFile('ICAO_ml_June_2022.ml')

# from https://www.bsi.bund.de/SharedDocs/Downloads/DE/BSI/ElekAusweise/CSCA/GermanMasterList.zip
unpackMlFile('20220623_DEMasterList.ml')

writeQrcFile()

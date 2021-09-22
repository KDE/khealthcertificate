#!/usr/bin/env python3
# SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
# SPDX-License-Identifier: LGPL-2.0-or-later

import argparse
import json
import requests


def downloadCdcFlatFile(url):
    req = requests.get(url)
    data = []
    for line in req.text.split('\n'):
        if not line:
            continue
        fields = line.split('|')
        fields = list(map(lambda x: x.strip(), fields))
        data.append(fields)
    return data;

parser = argparse.ArgumentParser(description='Download and merge HL7 CVX coding data.')
parser.add_argument('--output', type=str, required=True, help='Path to which the output should be written to')
arguments = parser.parse_args()

vacGroups = downloadCdcFlatFile('https://www2a.cdc.gov/vaccines/iis/iisstandards/downloads/VG.txt')
manufactures = downloadCdcFlatFile('https://www2a.cdc.gov/vaccines/iis/iisstandards/downloads/TRADENAME.txt')

manufacturesIdx = {}
for line in manufactures:
    if not line[3]:
        continue
    manufacturesIdx[line[2]] = line[3]

cvxData = {}
for line in vacGroups:
    cvx = {}
    cvx['n'] = line[0]
    cvx['d'] = line[3]
    if line[1] in manufacturesIdx:
        cvx['m'] = manufacturesIdx[line[1]]
    cvxData[line[1]]  = cvx;

f = open(arguments.output, 'w')
f.write(json.dumps(cvxData))
f.close()

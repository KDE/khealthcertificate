#!/usr/bin/env python3
# SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
# SPDX-License-Identifier: LGPL-2.0-or-later

import argparse
import csv
import json
import os
import requests
import zipfile

def writeToFile(fileName, content):
    f = open(fileName, 'wb')
    f.write(content)
    f.close();

def countDepth(n):
    depth = 0
    while n[2*depth:].startswith('- '):
        depth += 1
    return depth + 2

def findNodes(node, func):
    res = []
    if isinstance(node, list):
        for child in node:
            res += findNodes(child, func)
    else:
        if func(node):
            res.append(node)
        else:
            res += findNodes(node.get('children', []), func)
    return res;

def toCodeMap(node):
    l = []
    if isinstance(node, list):
        for child in node:
            l += toCodeMap(child)
    else:
        if 'code' in node:
            l.append((node['code'], node['name']))
        l += toCodeMap(node.get('children', []))
    return l


parser = argparse.ArgumentParser(description='Download and filter WHO ICD-11 data.')
parser.add_argument('--output', type=str, required=True, help='Path to which the output should be written to')
arguments = parser.parse_args()

# download WHO ICD-11 archive
icd11archive = 'simpletabulation.zip'
req = requests.get('https://icd.who.int/browse11/Downloads/Download?fileName=simpletabulation.zip')
writeToFile(icd11archive, req.content)

# unpack the xls file in there
zipFile = zipfile.ZipFile(icd11archive, 'r')
icd11xls = 'simpletabulation.xlsx'
writeToFile(icd11xls, zipFile.read(icd11xls))

# convert xls to csv
icd11csv = 'simpletabulation.csv'
os.system(f"libreoffice --headless --convert-to csv --infilter=CSV:44,34,76 {icd11xls}")
with open(icd11csv, newline='') as f:
    icd11table = list(csv.reader(f, delimiter=',', quotechar='"'))[1:]

# load ICD-11 tree
root = { 'children': [], 'kind': 'root', 'name': '' }
stack = [root]
for row in icd11table:
    kind = row[5];
    depth = countDepth(row[4])

    node = {}
    node['kind'] = kind
    node['name'] = row[4].lstrip('- ')
    if kind == 'category':
        node['code'] = row[2]
    elif kind == 'block':
        node['block'] = row[3]
    elif kind == 'chapter':
        node['chapter'] = row[8]
    else:
        continue
    if row[10] != 'True':
        node['children'] = []

    while len(stack) >= depth:
        stack.pop()
    parent = stack[-1]
    parent['children'].append(node)
    stack.append(node)

# extract relevant disease codes
# also: block == Virus?
diseases = findNodes(root, lambda n: n['kind'] == 'chapter' and (n['chapter'] == '01' or n['chapter'] == '25'))
diseaseCodes = toCodeMap(diseases)
diseaseCodes = list(filter(lambda entry: len(entry[0]) == 4, diseaseCodes)) # drop sub-categories
diseaseCodes.sort(key=lambda entry: entry[0])
print("Diseases: ", len(diseaseCodes))
writeToFile(os.path.join(arguments.output, 'diseases.json'), json.dumps(dict(diseaseCodes)).encode('utf-8'))

# extract relevant medication codes
vaccines = findNodes(root, lambda n: n['kind'] == 'block' and n['name'] == 'Vaccines')
vaccineCodes = toCodeMap(vaccines)
vaccineCodes.sort(key=lambda entry: entry[0])
print("Vaccines:", len(vaccineCodes))
writeToFile(os.path.join(arguments.output, 'vaccines.json'), json.dumps(dict(vaccineCodes)).encode('utf-8'))

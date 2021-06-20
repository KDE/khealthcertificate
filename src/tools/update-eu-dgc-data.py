#!/usr/bin/env python3
# SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
# SPDX-License-Identifier: LGPL-2.0-or-later

import value_sets_pb2

import argparse
import datetime
import io
import json
import os
import requests
import zipfile

value_sets = {}

def downloadValueSets(lang):
    valueSetsFile = f"value_sets_{lang}.zip"
    req = requests.get(f"https://svc90.main.px.t-online.de/version/v1/ehn-dgc/{lang}/value-sets")
    f = open(valueSetsFile, 'wb')
    f.write(req.content)
    f.close()

    zipFile = zipfile.ZipFile(valueSetsFile, 'r')
    binData = zipFile.read('export.bin')

    valueSets = value_sets_pb2.ValueSets()
    valueSets.ParseFromString(binData)

    for category in ['tg', 'vp', 'ma', 'mp', 'tcTt', 'tcMa', 'tcTr']:
        if not category in value_sets:
            value_sets[category] = {}
        stringMapping = getattr(valueSets, category)
        for entry in stringMapping.items:
            if lang == 'en':
                value_sets[category][entry.key] = entry.displayText;
            # only add translations if they actually differ
            elif value_sets[category][entry.key] != entry.displayText:
                value_sets[category][f"{entry.key}[{lang}]"] = entry.displayText;


parser = argparse.ArgumentParser(description='Download and merge translations for codes used in EU DGCs')
parser.add_argument('--output', type=str, required=True, help='Path to which the output should be written to')
arguments = parser.parse_args()

# keep en first
for lang in ['en', 'de']:
    downloadValueSets(lang)

for key in value_sets:
    f = open(os.path.join(arguments.output, f"{key}.json"), 'w')
    f.write(json.dumps(value_sets[key]))
    f.close()

#!/usr/bin/env python3
# SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
# SPDX-License-Identifier: LGPL-2.0-or-later

import value_sets_pb2

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

# keep en first
for lang in ['en', 'de']:
    downloadValueSets(lang)

print(json.dumps(value_sets))


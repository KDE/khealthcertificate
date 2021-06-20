/*
 * SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick 2.15
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.1 as QQC2
import org.kde.kirigami 2.12 as Kirigami
import org.kde.prison 1.0 as Prison
import org.kde.khealthcertificate 1.0

Kirigami.ScrollablePage {
    id: page
    title: "Vaccination Certificate"

    property var rawData
    property var cert

    Kirigami.FormLayout {
        width: parent.width

        Prison.Barcode {
            barcodeType: Prison.Barcode.QRCode
            content: rawData
            Kirigami.FormData.isSection: true
        }

        Kirigami.Separator {
            Kirigami.FormData.isSection: true
            Kirigami.FormData.label: "Person"
        }

        QQC2.Label {
            text: cert.name
            Kirigami.FormData.label: "Name:"
        }
        QQC2.Label {
            text: cert.dateOfBirth.toLocaleDateString(Qt.locale(), Locale.ShortFormat)
            visible: cert.dateOfBirth.getTime() != 0
            Kirigami.FormData.label: "Date of birth:"
        }

        Kirigami.Separator {
            Kirigami.FormData.isSection: true
            Kirigami.FormData.label: "Vaccination"
        }

        QQC2.Label {
            text: cert.date.toLocaleDateString(Qt.locale(), Locale.ShortFormat)
            Kirigami.FormData.label: "Date:"
        }
        QQC2.Label {
            text: cert.vaccine
            Kirigami.FormData.label: "Vaccine:"
        }
        QQC2.Label {
            text: cert.manufacturer
            Kirigami.FormData.label: "Manufacturer:"
        }
        QQC2.Label {
            text: cert.dose + "/" + cert.totalDoses
            Kirigami.FormData.label: "Dose:"
        }
    }
}

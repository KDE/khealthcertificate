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
    title: "Recovery Certificate"

    property var rawData
    property var cert

    ColumnLayout {
        width: parent.width

        Rectangle {
            id: validationBg
            Layout.fillWidth: true
            implicitHeight: barcode.implicitHeight + Kirigami.Units.largeSpacing * 4

            Prison.Barcode {
                anchors.centerIn: parent
                id: barcode
                barcodeType: Prison.Barcode.QRCode
                content: rawData
            }

            color: switch (cert.validationState) {
                case HealthCertificate.Valid: return Kirigami.Theme.positiveTextColor;
                case HealthCertificate.Partial: return Kirigami.Theme.neutralTextColor;
                case HealthCertificate.Invalid: return Kirigami.Theme.negativeTextColor;
                default: return "transparent"
            }
        }

        Kirigami.FormLayout {
            width: parent.width

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
                Kirigami.FormData.label: "Recovery"
            }

            QQC2.Label {
                text: cert.dateOfPositiveTest.toLocaleDateString(Qt.locale(), Locale.ShortFormat)
                Kirigami.FormData.label: "Positive test:"
            }
            QQC2.Label {
                text: cert.validFrom.toLocaleDateString(Qt.locale(), Locale.ShortFormat)
                Kirigami.FormData.label: "Valid from:"
            }
            QQC2.Label {
                text: cert.validUntil.toLocaleDateString(Qt.locale(), Locale.ShortFormat)
                Kirigami.FormData.label: "Valid until:"
            }
            QQC2.Label {
                text: cert.disease
                Kirigami.FormData.label: "Disease:"
            }

            Kirigami.Separator {
                Kirigami.FormData.isSection: true
                Kirigami.FormData.label: "Certificate"
            }

            QQC2.Label {
                text: cert.certificateIssuer
                Kirigami.FormData.label: "Issuer:"
            }
            QQC2.Label {
                text: cert.certificateId
                Kirigami.FormData.label: "Identifier:"
            }
        }
    }
}

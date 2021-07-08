/*
 * SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick 2.15
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.1 as QQC2
import org.kde.kirigami 2.12 as Kirigami
import org.kde.prison 1.0 as Prison
// //import org.kde.ki18n 1.0
import org.kde.khealthcertificate 1.0

Kirigami.ScrollablePage {
    id: page
    title: "Vaccination Certificate"

    property var cert

    function daysTo(d1, d2) {
        return (d2.getTime() - d1.getTime()) / (1000 * 60 * 60 * 24);
    }

    ColumnLayout {
        width: parent.width

        Rectangle {
            id: validationBg
            Layout.fillWidth: true
            implicitHeight: barcode.height + Kirigami.Units.largeSpacing * 4

            Prison.Barcode {
                anchors.centerIn: parent
                id: barcode
                barcodeType: Prison.Barcode.QRCode
                content: cert.rawData
                width: height
                height: minimumHeight * 3
            }

            color: switch (cert.validationState) {
                case HealthCertificate.Valid: return Kirigami.Theme.positiveTextColor;
                case HealthCertificate.Partial: return Kirigami.Theme.neutralTextColor;
                case HealthCertificate.Invalid: return Kirigami.Theme.negativeTextColor;
                default: return "transparent"
            }
        }

        Kirigami.FormLayout {
            Layout.fillWidth: true

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
                color: daysTo(cert.date, new Date()) >= 14 ? Kirigami.Theme.textColor : Kirigami.Theme.neutralTextColor
            }
            QQC2.Label {
                text: cert.disease
                Kirigami.FormData.label: "Disease:"
                visible: cert.disease
            }
            QQC2.Label {
                text: cert.vaccineType
                Kirigami.FormData.label: "Type:"
                visible: cert.vaccineType
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
                color: cert.dose < cert.totalDoses ? Kirigami.Theme.neutralTextColor : Kirigami.Theme.textColor
            }
            QQC2.Label {
                //text: KCountry.fromAlpha2(cert.country).emojiFlag + " " + KCountry.fromAlpha2(cert.country).name
                text: cert.country
                Kirigami.FormData.label: "Country:"
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
            QQC2.Label {
                text: cert.certificateIssueDate.toLocaleString(Qt.locale(), Locale.ShortFormat)
                Kirigami.FormData.label: "Issued:"
            }
            QQC2.Label {
                text: cert.certificateExpiryDate.toLocaleString(Qt.locale(), Locale.ShortFormat)
                Kirigami.FormData.label: "Expires:"
                visible: cert.certificateExpiryDate.getTime() != 0
            }
            QQC2.Label {
                text: cert.signatureState == HealthCertificate.ValidSignature ? "valid" : "invalid"
                visible: cert.signatureState != HealthCertificate.UnknownSignature
                Kirigami.FormData.label: "Signature:"
                color: cert.signatureState == HealthCertificate.InvalidSignature ? Kirigami.Theme.negativeTextColor : Kirigami.Theme.textColor
            }
        }
    }
}

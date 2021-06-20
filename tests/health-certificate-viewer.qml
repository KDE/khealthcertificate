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

Kirigami.ApplicationWindow {
    title: "Health Certificates"
    reachableModeEnabled: false

    width: 640
    height: 720

    pageStack.initialPage: mainPage

    Component {
        id: vaccinationPage
        VaccinationCertificatePage {}
    }
    Component {
        id: testPage
        TestCertificatePage {}
    }
    Component {
        id: recoveryPage
        RecoveryCertificatePage {}
    }

    Component {
        id: mainPage

        Kirigami.Page {
            title: "Health Certificate Viewer"

            function showCertificate(rawData) {
                var cert = HealthCertificateParser.parse(rawData);
                switch (cert.type) {
                    case HealthCertificate.Vaccination:
                        applicationWindow().pageStack.push(vaccinationPage, {"rawData": rawData, "cert": cert});
                        break;
                    case HealthCertificate.Test:
                        applicationWindow().pageStack.push(testPage, {"rawData": rawData, "cert": cert});
                        break;
                    case HealthCertificate.Recovery:
                        applicationWindow().pageStack.push(recoveryPage, {"rawData": rawData, "cert": cert});
                        break;
                    default:
                        console.log(cert);
                }
            }

            ColumnLayout {
                width: parent.width
                QQC2.Button {
                    Layout.fillWidth: true
                    text: "Example Vaccination"
                    onClicked: showCertificate("HC1:6BF+70790T9WJWG.FKY*4GO0.O1CV2 O5 N2FBBRW1*70HS8WY04AC*WIFN0AHCD8KD97TK0F90KECTHGWJC0FDC:5AIA%G7X+AQB9746HS80:54IBQF60R6$A80X6S1BTYACG6M+9XG8KIAWNA91AY%67092L4WJCT3EHS8XJC$+DXJCCWENF6OF63W5NW6WF6%JC QE/IAYJC5LEW34U3ET7DXC9 QE-ED8%E.JCBECB1A-:8$96646AL60A60S6Q$D.UDRYA 96NF6L/5QW6307KQEPD09WEQDD+Q6TW6FA7C466KCN9E%961A6DL6FA7D46JPCT3E5JDLA7$Q6E464W5TG6..DX%DZJC6/DTZ9 QE5$CB$DA/D JC1/D3Z8WED1ECW.CCWE.Y92OAGY8MY9L+9MPCG/D5 C5IA5N9$PC5$CUZCY$5Y$527B+A4KZNQG5TKOWWD9FL%I8U$F7O2IBM85CWOC%LEZU4R/BXHDAHN 11$CA5MRI:AONFN7091K9FKIGIY%VWSSSU9%01FO2*FTPQ3C3F");
                }

                QQC2.TextArea {
                    id: certEntry
                    Layout.fillWidth: true
                    text: "HC1:6BF+70790T9WJWG.FKY*4GO0.O1CV2 O5 N2FBBRW1*70HS8WY04AC*WIFN0AHCD8KD97TK0F90KECTHGWJC0FDC:5AIA%G7X+AQB9746HS80:54IBQF60R6$A80X6S1BTYACG6M+9XG8KIAWNA91AY%67092L4WJCT3EHS8XJC +DXJCCWENF6OF63W5NW6WF6%JC QE/IAYJC5LEW34U3ET7DXC9 QE-ED8%E.JCBECB1A-:8$96646AL60A60S6Q$D.UDRYA 96NF6L/5QW6307KQEPD09WEQDD+Q6TW6FA7C466KCN9E%961A6DL6FA7D46JPCT3E5JDLA7$Q6E464W5TG6..DX%DZJC6/DTZ9 QE5$CB$DA/D JC1/D3Z8WED1ECW.CCWE.Y92OAGY8MY9L+9MPCG/D5 C5IA5N9$PC5$CUZCY$5Y$527BHB6*L8ARHDJL.Q7*2T7:SCNFZN70H6*AS6+T$D9UCAD97R8NIBO+/RJVE$9PAGPTBIZEP MO-Q0:R13IURRQ5MV93M9V3X2U:NDZSF"
                }
                QQC2.Button {
                    Layout.fillWidth: true
                    text: "Load Certificate"
                    onClicked: showCertificate(certEntry.text)
                }
            }
        }
    }
}

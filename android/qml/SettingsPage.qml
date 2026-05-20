import QtQuick 2.15; import QtQuick.Controls 2.15; import QtQuick.Layouts 1.15
Page { title: "Settings"
    ColumnLayout { anchors.fill: parent; anchors.margins: 12; spacing: 10
        Label { text: "Settings"; font.pixelSize: 20; font.bold: true; color: "#00e5ff" }

        GroupBox { title: "Display"; Layout.fillWidth: true
            ColumnLayout {
                Switch { text: "Use hex IDs"; checked: true }
                Switch { text: "Color by CAN ID"; checked: true }
                Switch { text: "Show timestamps"; checked: false }
            }
        }

        GroupBox { title: "CAN"; Layout.fillWidth: true
            ColumnLayout {
                RowLayout { Label { text: "Default bitrate:"; color: "#8899aa" }
                    ComboBox { model: ["125k","250k","500k","1M"]; currentIndex: 2; Layout.preferredWidth: 80 }
                }
            }
        }

        GroupBox { title: "Data"; Layout.fillWidth: true
            ColumnLayout {
                Switch { text: "Save absolute timestamps"; checked: false }
                Switch { text: "Continuous logging"; checked: false }
                Label { text: "Max frames: 1000000"; color: "#667788"; font.pixelSize: 11 }
            }
        }

        GroupBox { title: "About"; Layout.fillWidth: true
            ColumnLayout {
                Label { text: "SavvyCAN\nVersion 223.0 (Android)\n\nBased on SavvyCAN by Collin Kidder\n(C) 2015-2024\n\nAndroid port: 2026\n\nLicensed under GPLv3"; color: "#8899aa"; font.pixelSize: 11 }
                Label { text: "Supported connections:\n• Bluetooth SPP (ESP32 CAN bridges)\n• WiFi TCP (ESP32 CAN bridges)\n• USB OTG (PEAK PCAN-USB)"; color: "#556677"; font.pixelSize: 10 }
            }
        }
    }
}

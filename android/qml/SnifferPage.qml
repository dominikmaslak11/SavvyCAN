import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Item {
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 10

        Label {
            text: "CAN Sniffer"
            font.pixelSize: 20
            font.bold: true
            color: "#00e5ff"
        }

        Label {
            text: "Watch for specific CAN frames and trigger on patterns.\n\nSet trigger conditions (ID, data pattern) and log matching frames."
            color: "#e0e0f0"
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
        }

        RowLayout {
            Label { text: "Trigger ID:"; color: "#8899aa" }
            TextField {
                id: snifferIdField
                placeholderText: "hex"
                color: "#e0e0f0"
                font.family: "monospace"
                Layout.fillWidth: true
            }
        }

        RowLayout {
            Label { text: "Data pattern:"; color: "#8899aa" }
            TextField {
                id: snifferPatternField
                placeholderText: "hex bytes"
                color: "#e0e0f0"
                font.family: "monospace"
                Layout.fillWidth: true
            }
        }

        Button {
            text: "Start Sniffing"
            Layout.fillWidth: true
            onClicked: statusLabel.text = "Sniffer started (watching for matching frames)."
        }

        ListView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            model: frameListModel
            delegate: ItemDelegate {
                height: 30
                width: parent ? parent.width : 200
                RowLayout {
                    anchors.fill: parent
                    spacing: 4
                    Label {
                        text: frameId || ""
                        color: "#00e5ff"
                        font.family: "monospace"
                        font.pixelSize: 11
                    }
                    Label {
                        text: frameData || ""
                        color: "#e0e0f0"
                        elide: Text.ElideRight
                        font.family: "monospace"
                        font.pixelSize: 10
                        Layout.fillWidth: true
                    }
                }
            }
        }

        Label {
            id: statusLabel
            text: "Sniffer ready."
            color: "#667788"
            font.pixelSize: 11
        }
    }
}

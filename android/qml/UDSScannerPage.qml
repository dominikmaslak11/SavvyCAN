import QtQuick 2.15; import QtQuick.Controls 2.15; import QtQuick.Layouts 1.15
Page { title: "UDS Scanner"
    ColumnLayout { anchors.fill: parent; anchors.margins: 12; spacing: 10
        Label { text: "UDS Scanner"; font.pixelSize: 20; font.bold: true; color: "#00e5ff" }
        Label { text: "Unified Diagnostic Services scanner.\n\nScan ECUs by sending UDS requests (0x7DF) and reading responses (0x7E8+)."; color: "#e0e0f0"; wrapMode: Text.WordWrap; Layout.fillWidth: true }
        Button { text: "Scan for ECUs"; Layout.fillWidth: true; onClicked: statusLabel.text = "UDS scan initiated..." }
        ListView { Layout.fillWidth: true; Layout.fillHeight: true; clip: true
            model: ListModel { id: ecuModel }
            delegate: ItemDelegate { height: 36; width: parent ? parent.width : 200
                RowLayout { anchors.fill: parent; spacing: 6
                    Label { text: model.id; color: "#00e5ff"; font.family: "monospace" }
                    Label { text: model.name; color: "#e0e0f0" }
                }
            }
        }
        Label { id: statusLabel; text: "Connect to CAN bus first."; color: "#667788"; font.pixelSize: 11 }
    }
}

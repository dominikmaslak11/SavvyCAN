import QtQuick 2.15; import QtQuick.Controls 2.15; import QtQuick.Layouts 1.15
Page { title: "DBC Editor"
    ColumnLayout { anchors.fill: parent; anchors.margins: 12; spacing: 10
        Label { text: "DBC Editor"; font.pixelSize: 20; font.bold: true; color: "#00e5ff" }
        Label { text: "View and edit DBC (CAN Database) files.\n\nDBC files define message IDs, signal names, bit positions, scaling, and units."; color: "#e0e0f0"; wrapMode: Text.WordWrap; Layout.fillWidth: true }
        Button { text: "Load DBC File..."; Layout.fillWidth: true; onClicked: statusLabel.text = "DBC file loading..." }
        Button { text: "Save DBC File..."; Layout.fillWidth: true; flat: true; onClicked: statusLabel.text = "DBC file saving..." }
        ListView { Layout.fillWidth: true; Layout.fillHeight: true; clip: true
            model: ListModel { id: dbcModel }
            delegate: ItemDelegate { height: 36; width: parent ? parent.width : 200
                RowLayout { anchors.fill: parent; spacing: 6
                    Label { text: "0x" + model.id; color: "#00e5ff"; font.family: "monospace" }
                    Label { text: model.name; color: "#e0e0f0"; Layout.fillWidth: true }
                    Label { text: model.dlc + "B"; color: "#8899aa" }
                }
            }
        }
        Label { id: statusLabel; text: "No DBC loaded."; color: "#667788"; font.pixelSize: 11 }
    }
}

import QtQuick 2.15; import QtQuick.Controls 2.15; import QtQuick.Layouts 1.15
Page { title: "ISOTP Interpreter"
    ColumnLayout { anchors.fill: parent; anchors.margins: 12; spacing: 10
        Label { text: "ISOTP Interpreter"; font.pixelSize: 20; font.bold: true; color: "#00e5ff" }
        Label { text: "ISO 15765-2 transport protocol decoder.\n\nReassembles multi-frame CAN messages into complete payloads.\n\nSupports extended addressing and flow control."; color: "#e0e0f0"; wrapMode: Text.WordWrap; Layout.fillWidth: true }
        Switch { id: isotpEnabled; text: "Enable ISOTP decoding" }
        ListView { Layout.fillWidth: true; Layout.fillHeight: true; clip: true
            model: ListModel { id: isotpModel }
            delegate: ItemDelegate { height: 44; width: parent ? parent.width : 200
                ColumnLayout { anchors.fill: parent; spacing: 1
                    Label { text: "ID: " + model.frameId + "  Len: " + model.length; color: "#00e5ff"; font.family: "monospace"; font.pixelSize: 11 }
                    Label { text: model.data; color: "#e0e0f0"; font.family: "monospace"; font.pixelSize: 10; elide: Text.ElideRight; Layout.fillWidth: true }
                }
            }
        }
        Label { text: "ISOTP decoder ready."; color: "#667788"; font.pixelSize: 11 }
    }
}

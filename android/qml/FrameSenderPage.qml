import QtQuick 2.15; import QtQuick.Controls 2.15; import QtQuick.Layouts 1.15
Page { title: "Frame Sender"
    ColumnLayout { anchors.fill: parent; anchors.margins: 12; spacing: 8
        Label { text: "Frame Sender"; font.pixelSize: 20; font.bold: true; color: "#00e5ff" }
        Label { text: "Send individual or repeating CAN frames."; color: "#e0e0f0" }
        RowLayout { Label { text: "ID:"; color: "#8899aa"; Layout.preferredWidth: 30 }
            TextField { id: sId; placeholderText: "hex"; color: "#e0e0f0"; font.family: "monospace"; Layout.fillWidth: true }
        }
        RowLayout { Label { text: "Data:"; color: "#8899aa"; Layout.preferredWidth: 30 }
            TextField { id: sData; placeholderText: "01 0C ..."; color: "#e0e0f0"; font.family: "monospace"; Layout.fillWidth: true }
        }
        RowLayout { Label { text: "Interval:"; color: "#8899aa"; Layout.preferredWidth: 30 }
            TextField { id: sInterval; placeholderText: "ms"; text: "1000"; color: "#e0e0f0"; Layout.preferredWidth: 70 }
            Label { text: "ms"; color: "#8899aa" }
            Label { text: "Count:"; color: "#8899aa" }
            TextField { id: sCount; placeholderText: "∞"; text: "0"; color: "#e0e0f0"; Layout.preferredWidth: 50 }
        }
        RowLayout { Label { text: "Bus:"; color: "#8899aa" }
            ComboBox { id: sBus; model: ["0","1","2","3"]; currentIndex: 0; Layout.preferredWidth: 60 }
        }
        Button { text: "Send Frame"; Layout.fillWidth: true
            onClicked: {
                var id = parseInt(sId.text, 16)
                if (isNaN(id)) return
                var raw = sData.text.replace(/[^0-9A-Fa-f]/g, '')
                var bytes = []
                for (var i = 0; i < raw.length; i += 2) { var b = parseInt(raw.substr(i,2),16); if (!isNaN(b)) bytes.push(b) }
                if (wifiManager.isConnected()) wifiManager.sendFrame(id, bytes, id > 0x7FF)
                else if (btManager.isConnected()) btManager.sendFrame(id, bytes, id > 0x7FF)
                else if (usbManager.isOpen()) usbManager.sendFrame(id, bytes, id > 0x7FF)
            }
        }
        Button { text: "Start Repeating"; Layout.fillWidth: true; flat: true }
        Button { text: "Stop"; Layout.fillWidth: true; flat: true }
    }
}

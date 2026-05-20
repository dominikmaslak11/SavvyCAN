import QtQuick 2.15; import QtQuick.Controls 2.15; import QtQuick.Layouts 1.15
Page { title: "Frame Playback"
    ColumnLayout { anchors.fill: parent; anchors.margins: 12; spacing: 10
        Label { text: "Frame Playback"; font.pixelSize: 20; font.bold: true; color: "#00e5ff" }
        Label { text: "Play back recorded CAN frames to the bus.\n\nSelect frames from the list and replay them with original or modified timing."; color: "#e0e0f0"; wrapMode: Text.WordWrap; Layout.fillWidth: true }
        Label { text: "Available frames: " + frameListModel.count; color: "#667788" }
        RowLayout { Label { text: "Speed:"; color: "#8899aa" }
            ComboBox { id: pbSpeed; model: ["0.5x","1x","2x","5x","10x"]; currentIndex: 1; Layout.preferredWidth: 80 }
        }
        RowLayout { Label { text: "Loop:"; color: "#8899aa" }
            Switch { id: pbLoop }
        }
        Button { text: "▶ Play"; Layout.fillWidth: true; onClicked: statusLabel.text = "Playback started..." }
        Button { text: "⏹ Stop"; Layout.fillWidth: true; flat: true; onClicked: statusLabel.text = "Playback stopped." }
        Label { id: statusLabel; text: "Ready."; color: "#667788"; font.pixelSize: 11 }
    }
}

import QtQuick 2.15; import QtQuick.Controls 2.15; import QtQuick.Layouts 1.15
Page { title: "CAN Bridge"
    ColumnLayout { anchors.fill: parent; anchors.margins: 12; spacing: 10
        Label { text: "CAN Bridge"; font.pixelSize: 20; font.bold: true; color: "#00e5ff" }
        Label { text: "Bridge CAN traffic between different buses or interfaces.\n\nForward frames from one CAN bus to another in real-time."; color: "#e0e0f0"; wrapMode: Text.WordWrap; Layout.fillWidth: true }
        RowLayout { Label { text: "From:"; color: "#8899aa"; Layout.preferredWidth: 50 }
            ComboBox { id: brFrom; model: ["BT","WiFi","USB","Bus 0","Bus 1"]; currentIndex: 0; Layout.fillWidth: true }
        }
        RowLayout { Label { text: "To:"; color: "#8899aa"; Layout.preferredWidth: 50 }
            ComboBox { id: brTo; model: ["BT","WiFi","USB","Bus 0","Bus 1"]; currentIndex: 1; Layout.fillWidth: true }
        }
        Button { text: "Start Bridge"; Layout.fillWidth: true; onClicked: statusLabel.text = "Bridge active: " + brFrom.currentText + " → " + brTo.currentText }
        Button { text: "Stop"; Layout.fillWidth: true; flat: true; onClicked: statusLabel.text = "Bridge stopped." }
        Label { id: statusLabel; text: "Bridge ready."; color: "#667788"; font.pixelSize: 11 }
    }
}

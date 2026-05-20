import QtQuick 2.15; import QtQuick.Controls 2.15; import QtQuick.Layouts 1.15
Page { title: "CAN Fuzzer"
    ColumnLayout { anchors.fill: parent; anchors.margins: 12; spacing: 10
        Label { text: "CAN Fuzzer"; font.pixelSize: 20; font.bold: true; color: "#ff5252" }
        Label { text: "⚠ Security research tool.\n\nSend randomized or pattern-based CAN frames to test ECU robustness.\nUse responsibly on your own hardware only."; color: "#e0e0f0"; wrapMode: Text.WordWrap; Layout.fillWidth: true }
        RowLayout { Label { text: "ID range:"; color: "#8899aa" }
            TextField { id: fuzzIdLo; placeholderText: "000"; color: "#e0e0f0"; font.family: "monospace"; Layout.preferredWidth: 70 }
            Label { text: "-"; color: "#8899aa" }
            TextField { id: fuzzIdHi; placeholderText: "7FF"; color: "#e0e0f0"; font.family: "monospace"; Layout.preferredWidth: 70 }
        }
        RowLayout { Label { text: "Interval (ms):"; color: "#8899aa" }
            TextField { id: fuzzInterval; text: "100"; color: "#e0e0f0"; Layout.preferredWidth: 70 }
        }
        Button { text: "Start Fuzzing"; Layout.fillWidth: true
            onClicked: statusLabel.text = "⚠ Fuzzing active! Sending random frames..."
        }
        Button { text: "Stop"; Layout.fillWidth: true; flat: true
            onClicked: statusLabel.text = "Fuzzing stopped."
        }
        Label { id: statusLabel; text: "Connect to CAN bus first."; color: "#ff5252"; font.pixelSize: 11 }
    }
}

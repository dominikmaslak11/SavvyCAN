import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Item {
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 10

        Label {
            text: "Graph View"
            font.pixelSize: 20
            font.bold: true
            color: "#00e5ff"
        }

        Label {
            text: "Real-time CAN signal graphing.\n\nSelect signals from a loaded DBC file and plot them against time.\n\nRequires: DBC file loaded with signal definitions."
            color: "#e0e0f0"
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
        }

        Button {
            text: "New Graph..."
            Layout.fillWidth: true
            onClicked: statusLabel.text = "Graphing module initializing..."
        }

        // Simple canvas placeholder
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "#0a0a1a"
            border.color: "#1a1a3a"
            radius: 4

            Label {
                anchors.centerIn: parent
                text: "Graph Canvas\n(Coming soon — Qt Charts integration)"
                color: "#445566"
                horizontalAlignment: Text.AlignHCenter
                font.pixelSize: 14
            }
        }

        Label {
            id: statusLabel
            text: "Frames received: " + frameListModel.count
            color: "#667788"
            font.pixelSize: 11
        }
    }
}

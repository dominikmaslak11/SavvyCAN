import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Item {
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 10

        Label {
            text: "File I/O"
            font.pixelSize: 20
            font.bold: true
            color: "#00e5ff"
        }

        Label {
            text: "Load CAN log files (GVRET, CSV, CRTD, BusMaster, Vector, PCAP, etc.)\n\nUse the Qt file dialog or Android share intent to open files."
            color: "#e0e0f0"
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
        }

        Button {
            text: "Load File (GVRET/CSV/CRTD)..."
            Layout.fillWidth: true
            onClicked: {
                // Android file picker - will be implemented via Qt file dialog
                statusLabel.text = "File loading requires Android file picker integration."
            }
        }

        Button {
            text: "Save Frames..."
            Layout.fillWidth: true
            onClicked: {
                statusLabel.text = "Saving requires Android storage permission."
            }
        }

        Label {
            id: statusLabel
            text: ""
            color: "#8899aa"
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
        }

        Label {
            text: "\nCurrent frames: " + frameListModel.count
            color: "#667788"
            font.pixelSize: 12
        }

        Label {
            text: "File formats supported:\n• GVRET (.gvret)\n• CSV (ID, D0-D7)\n• CRTD (OVMS)\n• BusMaster (.log)\n• Microchip (.log)\n• Vector Trace (.asc)\n• IXXAT Minilog\n• CAN-DO Logs\n• PCAN Viewer\n• SocketCAN PCAP"
            color: "#556677"
            font.pixelSize: 11
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
        }
    }
}

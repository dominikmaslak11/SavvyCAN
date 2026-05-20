import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Page {
    title: "Signal Viewer"
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 10
        Label { text: "Signal Viewer"; font.pixelSize: 20; font.bold: true; color: "#00e5ff" }
        Label { text: "Real-time decoded signal values from DBC definitions.\n\nLoad a DBC file, select a signal, and watch its value update as frames arrive."; color: "#e0e0f0"; wrapMode: Text.WordWrap; Layout.fillWidth: true }
        Rectangle { Layout.fillWidth: true; Layout.fillHeight: true; color: "#0a0a1a"; border.color: "#1a1a3a"; radius: 4
            Label { anchors.centerIn: parent; text: "Signal Viewer\n(DBC loading required)"; color: "#445566"; horizontalAlignment: Text.AlignHCenter; font.pixelSize: 14 }
        }
    }
}

import QtQuick 2.15; import QtQuick.Controls 2.15; import QtQuick.Layouts 1.15
Page { title: "File Compare"
    ColumnLayout { anchors.fill: parent; anchors.margins: 12; spacing: 10
        Label { text: "File Comparator"; font.pixelSize: 20; font.bold: true; color: "#00e5ff" }
        Label { text: "Compare two CAN log files and find differences.\n\nUseful for before/after comparison of CAN bus behavior."; color: "#e0e0f0"; wrapMode: Text.WordWrap; Layout.fillWidth: true }
        Button { text: "Load File A..."; Layout.fillWidth: true }
        Button { text: "Load File B..."; Layout.fillWidth: true; flat: true }
        Label { text: "Differences will appear here."; color: "#556677"; Layout.fillWidth: true }
        ListView { Layout.fillWidth: true; Layout.fillHeight: true; clip: true
            model: ListModel { id: diffModel }
            delegate: Label { text: model.line; color: "#ffab40"; font.family: "monospace"; font.pixelSize: 10 }
        }
    }
}

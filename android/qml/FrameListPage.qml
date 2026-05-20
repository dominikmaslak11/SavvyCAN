import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Item {
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 8
        spacing: 6

        // Stats bar
        RowLayout {
            Label {
                text: "Frames: " + frameListModel.count
                color: "#e0e0f0"
                font.family: "monospace"
                font.pixelSize: 13
            }
            Item { Layout.fillWidth: true }
            Switch {
                text: "Overwrite"
                checked: frameListModel.overwriteMode
                onCheckedChanged: frameListModel.setOverwriteMode(checked)
            }
            Button {
                text: "Clear"
                flat: true
                onClicked: frameStore.clearFrames()
            }
        }

        // Filter bar
        RowLayout {
            TextField {
                id: filterIdField
                placeholderText: "Filter ID (hex)"
                color: "#e0e0f0"
                font.family: "monospace"
                Layout.fillWidth: true
            }
            Button {
                text: "Filter"
                onClicked: {
                    var id = parseInt(filterIdField.text, 16)
                    if (!isNaN(id)) frameStore.setFilterState(id, true)
                }
            }
            Button {
                text: "All"
                flat: true
                onClicked: frameStore.setAllFilters(true)
            }
            Button {
                text: "None"
                flat: true
                onClicked: frameStore.setAllFilters(false)
            }
        }

        // Frame list
        ListView {
            id: frameList
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            model: frameListModel
            delegate: ItemDelegate {
                height: 36
                width: frameList.width
                RowLayout {
                    anchors.fill: parent
                    spacing: 4
                    Label {
                        text: frameId || "???"
                        color: extended ? "#ffab40" : "#00e5ff"
                        font.family: "monospace"
                        font.pixelSize: 13
                        Layout.preferredWidth: 68
                    }
                    Label {
                        text: frameData || ""
                        color: "#e0e0f0"
                        Layout.fillWidth: true
                        elide: Text.ElideRight
                        font.family: "monospace"
                        font.pixelSize: 10
                    }
                    Label {
                        text: (dlc || 0) + "B"
                        color: "#8899aa"
                        font.family: "monospace"
                        font.pixelSize: 10
                        Layout.preferredWidth: 24
                    }
                    Label {
                        text: isReceived ? "RX" : "TX"
                        color: isReceived ? "#4caf50" : "#ffab40"
                        font.family: "monospace"
                        font.pixelSize: 9
                    }
                }
            }
        }

        // Quick send
        RowLayout {
            TextField {
                id: quickSendId
                placeholderText: "ID"
                Layout.preferredWidth: 80
                color: "#e0e0f0"
                font.family: "monospace"
            }
            TextField {
                id: quickSendData
                placeholderText: "hex data"
                Layout.fillWidth: true
                color: "#e0e0f0"
                font.family: "monospace"
            }
            Button {
                text: "Send"
                onClicked: {
                    var id = parseInt(quickSendId.text, 16)
                    if (isNaN(id)) return
                    var raw = quickSendData.text.replace(/[^0-9A-Fa-f]/g, '')
                    var bytes = []
                    for (var i = 0; i < raw.length; i += 2) {
                        var b = parseInt(raw.substr(i, 2), 16)
                        if (!isNaN(b)) bytes.push(b)
                    }
                    var ext = id > 0x7FF
                    if (wifiManager.isConnected()) wifiManager.sendFrame(id, bytes, ext)
                    else if (btManager.isConnected()) btManager.sendFrame(id, bytes, ext)
                    else if (usbManager.isOpen()) usbManager.sendFrame(id, bytes, ext)
                }
            }
        }
    }
}

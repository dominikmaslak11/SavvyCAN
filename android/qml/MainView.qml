import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

ApplicationWindow {
    id: root
    visible: true
    width: 400
    height: 700
    title: "SavvyCAN"

    Material.theme: Material.Dark
    Material.accent: Material.Cyan

    // ── Connection Bar ─────────────────────────────────────────────────
    header: ToolBar {
        RowLayout {
            anchors.fill: parent
            ToolButton {
                text: "\u26A1"
                onClicked: connectionDrawer.open()
            }
            Label {
                text: "SavvyCAN"
                font.bold: true
                font.pixelSize: 18
                color: "#00e5ff"
                Layout.fillWidth: true
            }
            ToolButton {
                text: btManager.isConnected() ? "\uD83D\uDFE2" : "\uD83D\uDD34"
            }
        }
    }

    // ── Connection Drawer ──────────────────────────────────────────────
    Drawer {
        id: connectionDrawer
        width: 0.8 * root.width
        height: root.height

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 16
            spacing: 12

            Label {
                text: "Connection"
                font.pixelSize: 22
                font.bold: true
                color: "#00e5ff"
            }

            TabBar {
                id: tabBar
                Layout.fillWidth: true
                TabButton { text: "Bluetooth" }
                TabButton { text: "USB OTG" }
            }

            StackLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                currentIndex: tabBar.currentIndex

                // Bluetooth panel
                ColumnLayout {
                    Button {
                        text: "Scan Devices"
                        Layout.fillWidth: true
                        onClicked: btManager.startScan()
                    }
                    ListView {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        model: btDevices
                        delegate: ItemDelegate {
                            text: modelData
                            onClicked: btManager.connectToDevice(modelData)
                        }
                    }
                }

                // USB OTG panel
                ColumnLayout {
                    Label {
                        text: "Supported adapters:"
                        color: "#8899aa"
                    }
                    Repeater {
                        model: ["PEAK PCAN-USB", "PEAK PCAN-USB FD",
                                "PEAK PCAN-USB Pro", "PEAK PCAN-USB Pro FD"]
                        delegate: Button {
                            text: modelData
                            Layout.fillWidth: true
                            onClicked: usbManager.openDevice(0x0C72, 0x000C)
                        }
                    }
                }
            }
        }
    }

    // ── Frame List ─────────────────────────────────────────────────────
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 8

        RowLayout {
            Label {
                text: "Frames: " + (typeof frameCount !== 'undefined' ? frameCount : 0)
                color: "#e0e0f0"
            }
            Item { Layout.fillWidth: true }
        }

        ListView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            model: frameListModel
            delegate: ItemDelegate {
                height: 48
                RowLayout {
                    anchors.fill: parent
                    Label {
                        text: "0x" + (typeof frameId !== 'undefined' ? frameId.toString(16).toUpperCase() : "???")
                        color: "#00e5ff"
                        font.family: "monospace"
                    }
                    Label {
                        text: typeof frameData !== 'undefined' ? frameData : ""
                        color: "#e0e0f0"
                        Layout.fillWidth: true
                        elide: Text.ElideRight
                        font.family: "monospace"
                    }
                    Label {
                        text: (typeof dlc !== 'undefined' ? dlc : 0) + "B"
                        color: "#8899aa"
                        font.family: "monospace"
                    }
                }
            }
        }

        // Quick send bar
        RowLayout {
            TextField {
                id: sendIdField
                placeholderText: "ID (hex)"
                Layout.preferredWidth: 90
                color: "#e0e0f0"
                font.family: "monospace"
            }
            TextField {
                id: sendDataField
                placeholderText: "data hex"
                Layout.fillWidth: true
                color: "#e0e0f0"
                font.family: "monospace"
            }
            Button {
                text: "Send"
                onClicked: {
                    var id = parseInt(sendIdField.text, 16)
                    var parts = sendDataField.text.split(" ")
                    var bytes = []
                    for (var i = 0; i < parts.length; i++)
                        bytes.push(parseInt(parts[i], 16))
                    btManager.sendFrame(id, bytes, id > 0x7FF)
                }
            }
        }
    }
}

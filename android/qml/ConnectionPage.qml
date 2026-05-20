import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Item {
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 8

        Label {
            text: "Connection"
            font.pixelSize: 20
            font.bold: true
            color: "#00e5ff"
        }

        TabBar {
            id: connTabBar
            Layout.fillWidth: true
            TabButton { text: "Bluetooth" }
            TabButton { text: "Wi-Fi" }
            TabButton { text: "USB OTG" }
        }

        StackLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            currentIndex: connTabBar.currentIndex

            // ── Bluetooth ──────────────────────────────────────────────
            ColumnLayout {
                spacing: 10
                Button {
                    text: btManager.scanning ? "Scanning..." : "Paired Devices"
                    Layout.fillWidth: true
                    enabled: !btManager.scanning
                    onClicked: btManager.startScan()
                }
                ListView {
                    id: btDevList
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true
                    model: btDeviceListModel
                    delegate: ItemDelegate {
                        width: parent ? parent.width : 200
                        height: 50
                        onClicked: btManager.connectToDevice(address)
                        ColumnLayout {
                            anchors.verticalCenter: parent.verticalCenter
                            spacing: 2
                            Label {
                                text: name || address
                                color: "#e0e0f0"
                                font.pixelSize: 14
                            }
                            Label {
                                text: address
                                color: "#8899aa"
                                font.family: "monospace"
                                font.pixelSize: 11
                            }
                        }
                    }
                }
                Label {
                    visible: btDevList.count === 0
                    text: "Pair ESP32 bridge in Android Bluetooth settings first."
                    color: "#8899aa"
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
                }
                RowLayout {
                    Button {
                        text: btManager.isConnected() ? "Disconnect" : "Connect"
                        onClicked: btManager.isConnected() ? btManager.disconnectDevice() : {}
                        enabled: btManager.isConnected()
                    }
                    Label {
                        text: btManager.isConnected() ? "Connected" : "Disconnected"
                        color: btManager.isConnected() ? "#4caf50" : "#ff5252"
                    }
                }
            }

            // ── Wi-Fi ──────────────────────────────────────────────────
            ColumnLayout {
                spacing: 10
                TextField {
                    id: wifiIpField
                    placeholderText: "IP (e.g. 192.168.4.1)"
                    text: "192.168.4.1"
                    color: "#e0e0f0"
                    Layout.fillWidth: true
                }
                TextField {
                    id: wifiPortField
                    placeholderText: "Port"
                    text: "35000"
                    color: "#e0e0f0"
                    Layout.fillWidth: true
                }
                Button {
                    text: wifiManager.isConnected() ? "Disconnect" : "Connect"
                    Layout.fillWidth: true
                    onClicked: {
                        if (wifiManager.isConnected())
                            wifiManager.disconnectDevice()
                        else
                            wifiManager.connectToDevice(wifiIpField.text, parseInt(wifiPortField.text))
                    }
                }
                Label {
                    text: wifiManager.isConnected() ? "Connected" : "Disconnected"
                    color: wifiManager.isConnected() ? "#4caf50" : "#ff5252"
                }
            }

            // ── USB OTG ────────────────────────────────────────────────
            ColumnLayout {
                spacing: 10
                Label {
                    text: "Supported adapters:"
                    color: "#8899aa"
                }
                Repeater {
                    model: [
                        {n: "PEAK PCAN-USB", v: 0x0C72, p: 0x000C},
                        {n: "PCAN-USB FD", v: 0x0C72, p: 0x001C},
                        {n: "PCAN-USB Pro", v: 0x0C72, p: 0x000D},
                        {n: "PCAN-USB Pro FD", v: 0x0C72, p: 0x001D},
                        {n: "SLCAN (FTDI/CDC)", v: 0x0403, p: 0x6001}
                    ]
                    delegate: Button {
                        text: modelData.n
                        Layout.fillWidth: true
                        onClicked: usbManager.openDevice(modelData.v, modelData.p)
                    }
                }
                Label {
                    text: usbManager.isOpen() ? "Device opened" : "No USB device"
                    color: usbManager.isOpen() ? "#4caf50" : "#ff5252"
                }
                Button {
                    text: "Close USB"
                    enabled: usbManager.isOpen()
                    onClicked: usbManager.closeDevice()
                }
            }
        }
    }


    // BT device list model
    ListModel { id: btDeviceListModel }
    Connections {
        target: btManager
        function onDeviceFound(name, address) {
            for (var i = 0; i < btDeviceListModel.count; i++)
                if (btDeviceListModel.get(i).address === address) return
            btDeviceListModel.append({name: name, address: address})
        }
    }
}

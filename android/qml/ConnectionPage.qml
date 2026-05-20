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
            TabButton { text: "BLE CAN" }
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

            // ── BLE CAN Bridge ─────────────────────────────────────────
            ColumnLayout {
                spacing: 10
                Label {
                    text: "BLE CAN Bridge \u2014 ESP32 + MCP2515 250kbps"
                    color: "#00e5ff"
                    font.pixelSize: 13
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
                }

                // Scan
                Button {
                    text: bleManager.scanning ? "Skanowanie BLE..." : "Skanuj BLE"
                    Layout.fillWidth: true
                    enabled: !bleManager.scanning
                    onClicked: {
                        bleDeviceListModel.clear()
                        bleManager.startScan()
                    }
                }

                // Device list
                ListView {
                    id: bleDevList
                    Layout.fillWidth: true
                    Layout.preferredHeight: 140
                    clip: true
                    model: bleDeviceListModel
                    delegate: ItemDelegate {
                        width: parent ? parent.width : 200
                        height: 50
                        onClicked: bleManager.connectToDevice(address)
                        ColumnLayout {
                            anchors.verticalCenter: parent.verticalCenter
                            spacing: 2
                            Label {
                                text: name || address
                                color: "#e0e0f0"
                                font.pixelSize: 14
                            }
                            Label {
                                text: address + "  RSSI: " + rssi
                                color: "#8899aa"
                                font.family: "monospace"
                                font.pixelSize: 11
                            }
                        }
                    }
                }
                Label {
                    visible: bleDevList.count === 0
                    text: "Kliknij 'Skanuj BLE' aby znale\u017a\u0107 CANBridge"
                    color: "#8899aa"
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
                }

                // Connection status
                RowLayout {
                    Button {
                        text: bleManager.connected ? "Roz\u0142\u0105cz" : "Po\u0142\u0105cz"
                        enabled: bleManager.connected
                        onClicked: bleManager.disconnectDevice()
                    }
                    Label {
                        text: bleManager.connected ? "Po\u0142\u0105czony" : "Niepo\u0142\u0105czony"
                        color: bleManager.connected ? "#4caf50" : "#ff5252"
                    }
                }

                // LED + CAN status
                RowLayout {
                    spacing: 10
                    Button {
                        text: bleManager.ledOn ? "LED \u2b24 OFF" : "LED \u2b24 ON"
                        enabled: bleManager.connected
                        onClicked: bleManager.writeLed(!bleManager.ledOn)
                    }
                    Label {
                        text: "CAN frames: " + canFrameCount
                        color: "#8899aa"
                        font.pixelSize: 12
                    }
                }

                // Quick CAN send
                Rectangle {
                    color: "#1a1a2e"
                    radius: 6
                    Layout.fillWidth: true
                    Layout.preferredHeight: 100
                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 8
                        spacing: 4

                        RowLayout {
                            Label { text: "Send CAN:"; color: "#00e5ff"; font.pixelSize: 12 }
                            TextField {
                                id: canIdField
                                text: "123"
                                color: "#e0e0f0"
                                Layout.preferredWidth: 80
                                font.family: "monospace"
                            }
                            Switch {
                                id: canExtSwitch
                                text: "EXT"
                            }
                        }
                        RowLayout {
                            TextField {
                                id: canDataField
                                text: "DEADBEEF"
                                color: "#e0e0f0"
                                Layout.fillWidth: true
                                font.family: "monospace"
                            }
                            Button {
                                text: "Send"
                                enabled: bleManager.connected
                                onClicked: {
                                    var hex = canDataField.text.replace(/\\s/g, "")
                                    var bytes = []
                                    for (var i = 0; i + 1 < hex.length; i += 2)
                                        bytes.push(parseInt(hex.substr(i, 2), 16))
                                    var id = parseInt(canIdField.text, 16)
                                    if (!isNaN(id) && bytes.length > 0) {
                                        bleManager.sendCanFrame(id, canExtSwitch.checked, bytes)
                                        canFrameCount++
                                    }
                                }
                            }
                        }
                        Label {
                            text: "Format: ID(hex) + DATA(hex, max 16 znak\u00f3w)"
                            color: "#556"
                            font.pixelSize: 10
                        }
                    }
                }

                // CAN frame log
                ListView {
                    id: canFrameList
                    Layout.fillWidth: true
                    Layout.preferredHeight: 80
                    clip: true
                    model: canFrameListModel
                    delegate: Label {
                        text: model.text
                        color: "#c0c0d0"
                        font.family: "monospace"
                        font.pixelSize: 10
                    }
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

    // BLE device list model
    ListModel { id: bleDeviceListModel }
    Connections {
        target: bleManager
        function onDeviceFound(name, address, rssi) {
            for (var i = 0; i < bleDeviceListModel.count; i++)
                if (bleDeviceListModel.get(i).address === address) {
                    bleDeviceListModel.setProperty(i, "rssi", rssi)
                    return
                }
            bleDeviceListModel.append({name: name, address: address, rssi: rssi})
        }
        function onErrorOccurred(msg) { console.log("BLE error: " + msg) }
        function onCanFrameReceived(id, ext, data) {
            var hexId = id.toString(16).toUpperCase().padStart(ext ? 8 : 4, '0')
            var hexData = ""
            for (var i = 0; i < data.length; i++) {
                var b = data[i]
                if (b < 0) b += 256
                hexData += ("0" + b.toString(16).toUpperCase()).slice(-2)
            }
            var entry = (ext ? "T#" : "t#") + hexId + "#" + hexData
            canFrameListModel.insert(0, {text: entry})
            if (canFrameListModel.count > 50) canFrameListModel.remove(50)
            canFrameCount++
        }
    }

    property int canFrameCount: 0
    ListModel { id: canFrameListModel }
}
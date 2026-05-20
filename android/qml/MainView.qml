import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

ApplicationWindow {
    id: root
    visible: true
    width: 420
    height: 720
    title: "SavvyCAN"

    Material.theme: Material.Dark
    Material.accent: Material.Cyan

    // ── Status bar ────────────────────────────────────────────────────
    header: ToolBar {
        RowLayout {
            anchors.fill: parent
            ToolButton {
                text: "\u2630"  // ☰ menu
                onClicked: navDrawer.open()
            }
            Label {
                text: pageTitles[mainStack.currentIndex] || "SavvyCAN"
                font.bold: true
                font.pixelSize: 16
                color: "#00e5ff"
                Layout.fillWidth: true
            }
            ToolButton {
                text: {
                    if (btManager.isConnected()) return "\uD83D\uDFE2"
                    if (wifiManager.isConnected()) return "\uD83D\uDFE2"
                    if (usbManager.isOpen()) return "\uD83D\uDFE2"
                    return "\uD83D\uDD34"
                }
            }
        }
    }

    // ── Navigation Drawer ─────────────────────────────────────────────
    Drawer {
        id: navDrawer
        width: 0.75 * root.width
        height: root.height

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 12
            spacing: 4

            Label {
                text: "SavvyCAN"
                font.pixelSize: 20
                font.bold: true
                color: "#00e5ff"
                Layout.bottomMargin: 8
            }

            Label {
                text: "CAN Bus Analyzer"
                color: "#8899aa"
                font.pixelSize: 12
                Layout.bottomMargin: 12
            }

            // Navigation buttons
            Repeater {
                model: [
                    {icon: "\u26A1", title: "Connection", page: 0},
                    {icon: "\uD83D\uDCCB", title: "Frame List", page: 1},
                    {icon: "\uD83D\uDCC2", title: "File I/O", page: 2},
                    {icon: "\uD83D\uDCC8", title: "Graph View", page: 3},
                    {icon: "\uD83D\uDD0D", title: "Sniffer", page: 4},
                    {icon: "\uD83D\uDD21", title: "Signal Viewer", page: 5},
                    {icon: "\uD83D\uDD0C", title: "UDS Scanner", page: 6},
                    {icon: "\uD83E\uDDE9", title: "ISOTP", page: 7},
                    {icon: "\uD83D\uDCA5", title: "Fuzzing", page: 8},
                    {icon: "\uD83D\uDCE4", title: "Frame Sender", page: 9},
                    {icon: "\u25B6\uFE0F", title: "Playback", page: 10},
                    {icon: "\uD83D\uDD17", title: "CAN Bridge", page: 11},
                    {icon: "\uD83D\uDCC4", title: "DBC Editor", page: 12},
                    {icon: "\uD83D\uDD0E", title: "File Compare", page: 13},
                    {icon: "\u2699\uFE0F", title: "Settings", page: 14}
                ]
                delegate: ItemDelegate {
                    width: parent ? parent.width : 200
                    height: 44
                    onClicked: {
                        mainStack.currentIndex = modelData.page
                        navDrawer.close()
                    }
                    RowLayout {
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: 10
                        Label {
                            text: modelData.icon
                            font.pixelSize: 18
                        }
                        Label {
                            text: modelData.title
                            color: mainStack.currentIndex === modelData.page ? "#00e5ff" : "#e0e0f0"
                            font.bold: mainStack.currentIndex === modelData.page
                        }
                    }
                }
            }

            Item { Layout.fillHeight: true }

            // Status footer
            Label {
                text: "Frames: " + frameListModel.count
                color: "#667788"
                font.pixelSize: 11
            }
            Label {
                text: "v223.0 (Android)"
                color: "#556677"
                font.pixelSize: 10
            }
        }
    }

    // ── Page titles ───────────────────────────────────────────────────
    property var pageTitles: [
        "Connection", "Frame List", "File I/O", "Graph View",
        "Sniffer", "Signal Viewer", "UDS Scanner", "ISOTP",
        "Fuzzing", "Frame Sender", "Playback", "CAN Bridge",
        "DBC Editor", "File Compare", "Settings"
    ]

    // ── Main content stack ────────────────────────────────────────────
    StackLayout {
        id: mainStack
        anchors.fill: parent
        currentIndex: 0

        // Page 0: Connection
        ConnectionPage { }

        // Page 1: Frame List
        FrameListPage { }

        // Page 2: File I/O
        FileIOPage { }

        // Page 3: Graph View
        GraphPage { }

        // Page 4: Sniffer
        SnifferPage { }

        // Page 5: Signal Viewer
        SignalViewerPage { }

        // Page 6: UDS Scanner
        UDSScannerPage { }

        // Page 7: ISOTP
        ISOTPPage { }

        // Page 8: Fuzzing
        FuzzingPage { }

        // Page 9: Frame Sender
        FrameSenderPage { }

        // Page 10: Playback
        PlaybackPage { }

        // Page 11: CAN Bridge
        CANBridgePage { }

        // Page 12: DBC Editor
        DBCEditorPage { }

        // Page 13: File Compare
        FileComparePage { }

        // Page 14: Settings
        SettingsPage { }
    }
}

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QBluetoothPermission>
#include "BluetoothManager.h"
#include "UsbHostManager.h"
#include "framestore.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    app.setApplicationName("SavvyCAN");

    // ── Request Bluetooth permissions (Android 12+) ───────────────────
    QBluetoothPermission btPerm;
    btPerm.setCommunicationModes(QBluetoothPermission::Access);
    app.requestPermission(btPerm);

    // ── Core data store ───────────────────────────────────────────────
    FrameStore store;

    // ── Connection managers ───────────────────────────────────────────
    BluetoothManager btManager(&store);
    UsbHostManager usbManager(&store);

    // ── QML engine ────────────────────────────────────────────────────
    QQmlApplicationEngine engine;

    // Expose C++ objects to QML
    engine.rootContext()->setContextProperty("frameStore", &store);
    engine.rootContext()->setContextProperty("btManager", &btManager);
    engine.rootContext()->setContextProperty("usbManager", &usbManager);

    engine.load(QUrl("qrc:/android/qml/MainView.qml"));

    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}

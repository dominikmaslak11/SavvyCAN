#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "BluetoothManager.h"
#include "BleManager.h"
#include "UsbHostManager.h"
#include "WifiManager.h"
#include "FrameListModel.h"
#include "framestore.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    app.setApplicationName("SavvyCAN");

    // ── Core data store ───────────────────────────────────────────────
    FrameStore store;

    // ── QML models ────────────────────────────────────────────────────
    FrameListModel frameListModel(&store);

    // ── Connection managers ───────────────────────────────────────────
    BluetoothManager btManager(&store);
    BleManager        bleManager(&store);
    UsbHostManager usbManager(&store);
    WifiManager wifiManager(&store);

    // ── QML engine ────────────────────────────────────────────────────
    QQmlApplicationEngine engine;

    // Expose C++ objects to QML
    engine.rootContext()->setContextProperty("frameStore", &store);
    engine.rootContext()->setContextProperty("frameListModel", &frameListModel);
    engine.rootContext()->setContextProperty("btManager", &btManager);
    engine.rootContext()->setContextProperty("bleManager", &bleManager);
    engine.rootContext()->setContextProperty("usbManager", &usbManager);
    engine.rootContext()->setContextProperty("wifiManager", &wifiManager);

    const QUrl url("qrc:/android/qml/MainView.qml");
    engine.load(url);

    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
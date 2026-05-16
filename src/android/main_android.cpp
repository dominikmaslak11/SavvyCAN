#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "BluetoothManager.h"
#include "UsbHostManager.h"
#include "framestore.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    app.setApplicationName("SavvyCAN");

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

    const QUrl url("qrc:/android/qml/MainView.qml");
    engine.load(url);

    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}

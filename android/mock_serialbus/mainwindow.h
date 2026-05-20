#ifndef STUB_MAINWINDOW_H
#define STUB_MAINWINDOW_H

#include <QObject>
#include <QString>
#include "canframemodel.h"
#include "framestore.h"
#include "dbc/dbchandler.h"

// Minimal stub for MainWindow - provides static accessors used by bus_protocols and other code.
// On Android, the QML UI replaces the desktop QMainWindow.

class MainWindow : public QObject
{
    Q_OBJECT
public:
    static MainWindow *getReference() {
        static MainWindow instance;
        return &instance;
    }

    CANFrameModel *getCANFrameModel() { return &mModel; }
    FrameStore *getFrameStore() { return &mStore; }
    DBCHandler *getDBCHandler() { return DBCHandler::getReference(); }

    static QString loadedFileName;

private:
    MainWindow() {}
    CANFrameModel mModel{this};  // uses CANFrameModel(QObject *parent)
    FrameStore mStore;
};

#endif // STUB_MAINWINDOW_H

#include <QtTest>
#include <QGuiApplication>
#include "dbc_classes.h"
#include "dbchandler.h"

class DBCTest : public QObject
{
    Q_OBJECT

private slots:
    void testDBCMessageDefaults()
    {
        DBC_MESSAGE msg;
        QCOMPARE(msg.ID, 0u);
        QVERIFY(msg.name.isEmpty());
    }

    void testDBCSignalDefaults()
    {
        DBC_SIGNAL sig;
        // startBit defaults to 1
        // signalSize default varies;
        QVERIFY(sig.name.isEmpty());
    }

    void testDBCNodeDefaults()
    {
        DBC_NODE node;
        QVERIFY(node.name.isEmpty());
        QVERIFY(node.comment.isEmpty());
    }

    void testDBCHandlerDefaults()
    {
        DBCHandler *handler = DBCHandler::getReference();
        QVERIFY(handler->getFileCount() == 0);
    }
};

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    DBCTest test;
    return QTest::qExec(&test, argc, argv);
}
#include "test_dbc.moc"

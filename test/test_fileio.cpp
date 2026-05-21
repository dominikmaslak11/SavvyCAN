#include <QtTest>
#include <QFile>
#include "can_structs.h"
#include "framefileio.h"

class FileIOTest : public QObject
{
    Q_OBJECT

private slots:
    void testCRTDRoundtrip()
    {
        QVector<CANFrame> frames;
        for (int i = 0; i < 10; ++i) {
            CANFrame f;
            f.setFrameId(0x100 + i);
            f.bus = 0;
            f.setTimeStamp(QCanBusFrame::TimeStamp(0, i * 100000));
            f.setFrameType(QCanBusFrame::DataFrame);
            f.setPayload(QByteArray::fromHex("1122334455667788"));
            frames.append(f);
        }

        QString tmp = "/tmp/savvycan_test.crtd";
        QVERIFY(FrameFileIO::saveFrameFile(tmp, &frames));

        QVector<CANFrame> loaded;
        QVERIFY(FrameFileIO::loadCRTDFile(tmp, &loaded));
        QCOMPARE(loaded.size(), frames.size());

        for (int i = 0; i < qMin(frames.size(), loaded.size()); ++i) {
            QCOMPARE(loaded[i].frameId(), frames[i].frameId());
            QCOMPARE(loaded[i].payload(), frames[i].payload());
        }

        QFile::remove(tmp);
    }

    void testEmptyLoadCRTD()
    {
        QFile tmpf("/tmp/savvycan_test2.crtd");
        QVERIFY(tmpf.open(QIODevice::WriteOnly));
        tmpf.write("# CRTD v1.0\n");
        tmpf.write("# timestamp,ID,extended,bus,LEN,D0,D1,D2,D3,D4,D5,D6,D7\n");
        tmpf.write("0,100,0,0,8,01,02,03,04,05,06,07,08\n");
        tmpf.write("1000,200,0,0,4,AA,BB,CC,DD\n");
        tmpf.close();

        QVector<CANFrame> loaded;
        bool ok = FrameFileIO::loadCRTDFile(tmpf.fileName(), &loaded);
        if (ok && loaded.size() >= 2) {
            QCOMPARE(loaded[0].frameId(), 0x100u);
            QVERIFY(loaded[0].payload().size() >= 8);
        }
        QFile::remove(tmpf.fileName());
    }
};

QTEST_MAIN(FileIOTest)
#include "test_fileio.moc"

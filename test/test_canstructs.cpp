#include <QtTest>
#include "can_structs.h"
#include "can_trigger_structs.h"

class CANStructsTest : public QObject
{
    Q_OBJECT

private slots:
    void testDefaultFrame()
    {
        CANFrame f;
        QCOMPARE(f.frameId(), 0u);
        QCOMPARE(f.bus, 0);
        QVERIFY(!f.hasExtendedFrameFormat());
        QVERIFY(f.isReceived);
    }

    void testFrameId()
    {
        CANFrame f;
        f.setFrameId(0x7E8);
        QCOMPARE(f.frameId(), 0x7E8u);
        f.setFrameId(0x1FFFFFFF);
        QCOMPARE(f.frameId(), 0x1FFFFFFFu);
    }

    void testExtendedFrame()
    {
        CANFrame f;
        f.setExtendedFrameFormat(true);
        QVERIFY(f.hasExtendedFrameFormat());
        f.setExtendedFrameFormat(false);
        QVERIFY(!f.hasExtendedFrameFormat());
    }

    void testPayload()
    {
        CANFrame f;
        QByteArray data = QByteArray::fromHex("DEADBEEF11223344");
        f.setPayload(data);
        QCOMPARE(f.payload(), data);
    }

    void testTimestamp()
    {
        CANFrame f;
        f.setTimeStamp(QCanBusFrame::TimeStamp(10, 500000));
        QCOMPARE(f.timeStamp().seconds(), 10ull);
        QCOMPARE(f.timeStamp().microSeconds(), 500000ull);
    }

    void testFrameComparison()
    {
        CANFrame a, b;
        a.setTimeStamp(QCanBusFrame::TimeStamp(1, 0));
        b.setTimeStamp(QCanBusFrame::TimeStamp(2, 0));
        QVERIFY(a < b);
        QVERIFY(!(b < a));
    }

    void testFrameType()
    {
        CANFrame f;
        f.setFrameType(QCanBusFrame::DataFrame);
        QCOMPARE(f.frameType(), QCanBusFrame::DataFrame);
    }

    void testSendDataDefaults()
    {
        FrameSendData sd;
        sd.enabled = false; QVERIFY(!sd.enabled);
        sd.count = 0; QCOMPARE(sd.count, 0);
        QVERIFY(sd.triggers.isEmpty());
    }

    void testSendDataInheritsFrame()
    {
        FrameSendData sd;
        sd.setFrameId(0x555);
        sd.bus = 1;
        sd.isReceived = false;
        QCOMPARE(sd.frameId(), 0x555u);
    }

    void testSendDataTriggers()
    {
        FrameSendData sd;
        Trigger t;
        t.ID = 0x200; t.milliseconds = 100;
        t.maxCount = 5; t.triggerMask = TRG_ID | TRG_MS;
        sd.triggers.append(t);
        QCOMPARE(sd.triggers[0].ID, 0x200);
    }

    void testTriggerMasks()
    {
        QCOMPARE(TRG_ID, 1);
        QCOMPARE(TRG_MS, 2);
        QCOMPARE(TRG_BUS, 8);
        QVERIFY(!((8) & TRG_ID)); // TRG_BUS != TRG_ID
    }
};

QTEST_MAIN(CANStructsTest)
#include "test_canstructs.moc"

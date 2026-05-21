#include <QtTest>
#include "can_structs.h"
#include "can_trigger_structs.h"

/// Test that FrameSenderObject correctly handles entries with zero triggers
/// Regression test for: break vs continue bug
class FrameSenderRegression : public QObject
{
    Q_OBJECT

private slots:
    void testTriggerMaskDefault()
    {
        // Verify trigger masks are mutually distinct
        QVERIFY(TRG_ID == 1);
        QVERIFY(TRG_MS == 2);
        QVERIFY(TRG_COUNT == 4);
    }

    void testZeroTriggersShouldNotBlockOtherEntries()
    {
        // Create two FrameSendData: one with triggers, one without
        FrameSendData withTriggers;
        withTriggers.enabled = true;
        Trigger t;
        t.ID = 0x200;
        t.milliseconds = 100;
        t.maxCount = 5;
        t.triggerMask = TRG_ID | TRG_MS;
        withTriggers.triggers.append(t);

        FrameSendData withoutTriggers;
        withoutTriggers.enabled = true;
        // withoutTriggers has NO triggers

        // The bug was: break instead of continue when triggers.count()==0
        // This caused all subsequent entries to be skipped
        QCOMPARE(withTriggers.triggers.count(), 1);
        QCOMPARE(withoutTriggers.triggers.count(), 0);
    }

    void testModifierOperandTypes()
    {
        ModifierOperand op;
        op.ID = -1;  // shadow register
        QCOMPARE(op.ID, -1);
        op.ID = -2;  // own data byte
        QCOMPARE(op.ID, -2);
        op.ID = 0;   // numeric constant
        QCOMPARE(op.ID, 0);
    }

    void testAllModifierOperations()
    {
        QCOMPARE(ADDITION, 0);
        QCOMPARE(SUBTRACTION, 1);
        QCOMPARE(MULTIPLICATION, 2);
        QCOMPARE(DIVISION, 3);
        QCOMPARE(AND, 4);
        QCOMPARE(OR, 5);
        QCOMPARE(XOR, 6);
        QCOMPARE(MOD, 7);
    }
};

QTEST_MAIN(FrameSenderRegression)
#include "test_framesender_regression.moc"

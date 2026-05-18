#ifndef LIN_SCHEDULE_VIEW_H
#define LIN_SCHEDULE_VIEW_H

#include <QDialog>
#include <QTableWidget>
#include <QPushButton>
#include <QSpinBox>
#include "lin_structs.h"

// ═══════════════════════════════════════════════════════════════════════════
// LINScheduleView — editor and viewer for the LIN schedule table
// ═══════════════════════════════════════════════════════════════════════════

class LINScheduleView : public QDialog
{
    Q_OBJECT

public:
    explicit LINScheduleView(QWidget *parent = nullptr);
    virtual ~LINScheduleView();

    void setSchedule(const QVector<LINScheduleEntry> &schedule);
    QVector<LINScheduleEntry> schedule() const;

    void setLDFDatabase(const LDFDatabase *db);

signals:
    void scheduleChanged();

private slots:
    void onAddEntry();
    void onRemoveEntry();
    void onClear();
    void onCellChanged(int row, int col);

private:
    void setupUI();
    void refreshTable();

    QTableWidget *mTable = nullptr;
    QPushButton  *mAddBtn = nullptr;
    QPushButton  *mRemoveBtn = nullptr;
    QPushButton  *mClearBtn = nullptr;

    QVector<LINScheduleEntry> mEntries;
    const LDFDatabase *mLDFDB = nullptr;
};

#endif // LIN_SCHEDULE_VIEW_H

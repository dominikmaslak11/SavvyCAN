#include "lin_schedule_view.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>

LINScheduleView::LINScheduleView(QWidget *parent)
    : QDialog(parent)
{
    setupUI();
}

LINScheduleView::~LINScheduleView() {}

void LINScheduleView::setupUI()
{
    setWindowTitle(tr("LIN Schedule Editor"));
    resize(600, 400);

    auto *mainLayout = new QVBoxLayout(this);

    // Table
    mTable = new QTableWidget(0, 4, this);
    mTable->setHorizontalHeaderLabels({
        tr("Frame ID"), tr("Name"), tr("Delay (ms)"), tr("Active")
    });
    mTable->horizontalHeader()->setStretchLastSection(true);
    mTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    mainLayout->addWidget(mTable);

    // Buttons
    auto *btnLayout = new QHBoxLayout();
    mAddBtn = new QPushButton(tr("Add Entry"));
    mRemoveBtn = new QPushButton(tr("Remove Entry"));
    mClearBtn = new QPushButton(tr("Clear All"));

    btnLayout->addWidget(mAddBtn);
    btnLayout->addWidget(mRemoveBtn);
    btnLayout->addWidget(mClearBtn);
    btnLayout->addStretch();

    mainLayout->addLayout(btnLayout);

    // Connections
    connect(mAddBtn, &QPushButton::clicked, this, &LINScheduleView::onAddEntry);
    connect(mRemoveBtn, &QPushButton::clicked, this, &LINScheduleView::onRemoveEntry);
    connect(mClearBtn, &QPushButton::clicked, this, &LINScheduleView::onClear);
    connect(mTable, &QTableWidget::cellChanged, this, &LINScheduleView::onCellChanged);
}

void LINScheduleView::setSchedule(const QVector<LINScheduleEntry> &schedule)
{
    mEntries = schedule;
    refreshTable();
}

QVector<LINScheduleEntry> LINScheduleView::schedule() const
{
    return mEntries;
}

void LINScheduleView::setLDFDatabase(const LDFDatabase *db)
{
    mLDFDB = db;
}

void LINScheduleView::refreshTable()
{
    mTable->blockSignals(true);
    mTable->setRowCount(0);

    for (int i = 0; i < mEntries.size(); ++i) {
        const auto &entry = mEntries[i];
        mTable->insertRow(i);

        // Frame ID
        auto *idItem = new QTableWidgetItem(QString::number(entry.frameId));
        mTable->setItem(i, 0, idItem);

        // Name
        auto *nameItem = new QTableWidgetItem(entry.frameName);
        mTable->setItem(i, 1, nameItem);

        // Delay
        auto *delayItem = new QTableWidgetItem(QString::number(entry.delay_ms));
        mTable->setItem(i, 2, delayItem);

        // Active checkbox
        auto *activeItem = new QTableWidgetItem();
        activeItem->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
        activeItem->setCheckState(entry.active ? Qt::Checked : Qt::Unchecked);
        mTable->setItem(i, 3, activeItem);
    }

    mTable->blockSignals(false);
}

void LINScheduleView::onAddEntry()
{
    LINScheduleEntry entry;
    entry.frameId = 0;
    entry.frameName = QString("Frame_%1").arg(mEntries.size());
    entry.delay_ms = 50;
    entry.active = true;
    mEntries.append(entry);
    refreshTable();
    emit scheduleChanged();
}

void LINScheduleView::onRemoveEntry()
{
    int row = mTable->currentRow();
    if (row >= 0 && row < mEntries.size()) {
        mEntries.removeAt(row);
        refreshTable();
        emit scheduleChanged();
    }
}

void LINScheduleView::onClear()
{
    mEntries.clear();
    refreshTable();
    emit scheduleChanged();
}

void LINScheduleView::onCellChanged(int row, int col)
{
    if (row < 0 || row >= mEntries.size()) return;

    auto *item = mTable->item(row, col);
    if (!item) return;

    bool ok;
    switch (col) {
    case 0: // ID
        mEntries[row].frameId = (uint8_t)item->text().toUInt(&ok);
        break;
    case 1: // Name
        mEntries[row].frameName = item->text();
        break;
    case 2: // Delay
        mEntries[row].delay_ms = item->text().toUInt(&ok);
        break;
    case 3: // Active
        mEntries[row].active = (item->checkState() == Qt::Checked);
        break;
    }

    emit scheduleChanged();
}

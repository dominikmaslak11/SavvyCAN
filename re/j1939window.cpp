#include "j1939window.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QHeaderView>

J1939Window::J1939Window(QWidget *parent)
    : QDialog(parent)
{
    setupUI();

    connect(&mHandler, &J1939Handler::pgnDecoded, this, &J1939Window::onPGNDecoded);
    connect(&mHandler, &J1939Handler::dtcUpdated, this, &J1939Window::onDTCUpdated);
    connect(&mHandler, &J1939Handler::transportComplete, this, &J1939Window::onTransportComplete);
}

J1939Window::~J1939Window() {}

void J1939Window::setupUI()
{
    setWindowTitle(tr("J1939 Decoder"));
    resize(700, 500);

    auto *mainLayout = new QVBoxLayout(this);

    // Status
    mStatusLabel = new QLabel(tr("Frames: 0 | PGNs decoded: 0"));
    mainLayout->addWidget(mStatusLabel);

    // PGN Table
    mPGNTable = new QTableWidget(0, 4, this);
    mPGNTable->setHorizontalHeaderLabels({
        tr("PGN"), tr("Name"), tr("SPN"), tr("Value")
    });
    mPGNTable->horizontalHeader()->setStretchLastSection(true);
    mPGNTable->setAlternatingRowColors(true);
    mainLayout->addWidget(mPGNTable, 3);

    // DTC Table
    mDTCTable = new QTableWidget(0, 5, this);
    mDTCTable->setHorizontalHeaderLabels({
        tr("SPN"), tr("FMI"), tr("Description"), tr("Count"), tr("Lamp")
    });
    mDTCTable->horizontalHeader()->setStretchLastSection(true);
    mDTCTable->setAlternatingRowColors(true);
    mainLayout->addWidget(mDTCTable, 2);

    // Buttons
    auto *btnLayout = new QHBoxLayout();
    auto *clearBtn = new QPushButton(tr("Clear"));
    auto *autoScroll = new QPushButton(tr("Auto Scroll"));
    autoScroll->setCheckable(true);
    autoScroll->setChecked(true);
    btnLayout->addWidget(clearBtn);
    btnLayout->addWidget(autoScroll);
    btnLayout->addStretch();

    mainLayout->addLayout(btnLayout);

    connect(clearBtn, &QPushButton::clicked, this, &J1939Window::clear);
    connect(autoScroll, &QPushButton::toggled, this, [this](bool on) {
        if (on) mPGNTable->scrollToBottom();
    });
}

void J1939Window::processFrame(const CANFrame &frame)
{
    if (!frame.hasExtendedFrameFormat()) return;
    mFrameCount++;
    mHandler.processFrame(frame);
}

void J1939Window::clear()
{
    mPGNTable->setRowCount(0);
    mDTCTable->setRowCount(0);
    mFrameCount = 0;
    mStatusLabel->setText(tr("Frames: 0 | PGNs decoded: 0"));
}

void J1939Window::onPGNDecoded(uint32_t pgn, QMap<uint32_t, double> values)
{
    const J1939_PGN *pgdef = mHandler.database().findPGN(pgn);
    QString pgnName = pgdef ? pgdef->name : QString("0x%1").arg(pgn, 5, 16, QChar('0'));

    for (auto it = values.begin(); it != values.end(); ++it) {
        int row = mPGNTable->rowCount();
        mPGNTable->insertRow(row);

        mPGNTable->setItem(row, 0, new QTableWidgetItem(QString("0x%1").arg(pgn, 5, 16, QChar('0'))));
        mPGNTable->setItem(row, 1, new QTableWidgetItem(pgnName));

        const J1939_SPN *spdef = mHandler.database().findSPN(it.key());
        QString spnName = spdef ? spdef->name : QString("SPN %1").arg(it.key());
        mPGNTable->setItem(row, 2, new QTableWidgetItem(spnName));

        QString valueStr;
        if (spdef) {
            valueStr = QString::number(it.value(), 'f', 2) + " " + spdef->unit;
        } else {
            valueStr = QString::number(it.value(), 'f', 2);
        }
        mPGNTable->setItem(row, 3, new QTableWidgetItem(valueStr));
    }

    mStatusLabel->setText(tr("Frames: %1 | PGNs decoded: %2")
        .arg(mFrameCount).arg(mPGNTable->rowCount()));

    if (mPGNTable->rowCount() > 1000)
        mPGNTable->removeRow(0);

    mPGNTable->scrollToBottom();
}

void J1939Window::onDTCUpdated(const QVector<J1939_DTC> &dtcs)
{
    for (const auto &dtc : dtcs) {
        int row = mDTCTable->rowCount();
        mDTCTable->insertRow(row);

        mDTCTable->setItem(row, 0, new QTableWidgetItem(QString::number(dtc.spn)));
        mDTCTable->setItem(row, 1, new QTableWidgetItem(QString::number(dtc.fmi)));
        mDTCTable->setItem(row, 2, new QTableWidgetItem(dtc.fmiDescription()));
        mDTCTable->setItem(row, 3, new QTableWidgetItem(QString::number(dtc.occurrence)));
        mDTCTable->setItem(row, 4, new QTableWidgetItem(QString("0x%1").arg(dtc.lampStatus, 2, 16, QChar('0'))));
    }
}

void J1939Window::onTransportComplete(uint32_t pgn, const QByteArray &data)
{
    Q_UNUSED(pgn)
    Q_UNUSED(data)
    // Multi-frame messages will be decoded in onPGNDecoded when they complete
}

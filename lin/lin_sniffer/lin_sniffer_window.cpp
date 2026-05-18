#include "lin_sniffer_window.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QCheckBox>
#include <QFont>

LINSnifferWindow::LINSnifferWindow(QWidget *parent)
    : QDialog(parent)
{
    setupUI();
}

LINSnifferWindow::~LINSnifferWindow() {}

void LINSnifferWindow::setupUI()
{
    setWindowTitle(tr("LIN Sniffer"));
    resize(800, 500);

    auto *mainLayout = new QVBoxLayout(this);

    // Toolbar
    auto *toolbar = new QHBoxLayout();
    auto *clearBtn = new QPushButton(tr("Clear"));
    auto *pauseBtn = new QPushButton(tr("Pause"));
    pauseBtn->setCheckable(true);
    auto *tsCheck = new QCheckBox(tr("Timestamps"));
    tsCheck->setChecked(true);
    auto *parityCheck = new QCheckBox(tr("Parity"));
    parityCheck->setChecked(true);

    toolbar->addWidget(clearBtn);
    toolbar->addWidget(pauseBtn);
    toolbar->addStretch();
    toolbar->addWidget(tsCheck);
    toolbar->addWidget(parityCheck);
    toolbar->addStretch();

    mStatusLabel = new QLabel(tr("Frames: 0"));
    toolbar->addWidget(mStatusLabel);

    mainLayout->addLayout(toolbar);

    // Text display
    mTextDisplay = new QTextEdit();
    mTextDisplay->setReadOnly(true);
    mTextDisplay->setFont(QFont("Courier New", 9));
    mTextDisplay->setLineWrapMode(QTextEdit::NoWrap);
    mainLayout->addWidget(mTextDisplay);

    // Connections
    connect(clearBtn, &QPushButton::clicked, this, &LINSnifferWindow::clear);
    connect(pauseBtn, &QPushButton::toggled, this, [this](bool checked) { mPaused = checked; });
    connect(tsCheck, &QCheckBox::toggled, this, &LINSnifferWindow::setShowTimestamps);
    connect(parityCheck, &QCheckBox::toggled, this, &LINSnifferWindow::setShowParity);
}

QString LINSnifferWindow::formatFrame(const LINFrame &frame) const
{
    QString line;

    if (mShowTimestamps) {
        line += QString("%1 ").arg(frame.timestamp, 10, 10, QChar('0'));
    }

    if (mShowParity) {
        line += QString("PID:0x%1 ").arg(frame.pid, 2, 16, QChar('0'));
    }

    line += QString("ID:0x%1 ").arg(frame.id, 2, 16, QChar('0'));

    // Direction
    line += frame.isReceived ? "Rx " : "Tx ";

    // Data
    line += QString("DLC:%1 ").arg(frame.dataLen);
    line += frame.dataHex();

    // Checksum
    line += QString("  CS:0x%1").arg(frame.checksum, 2, 16, QChar('0'));
    if (frame.checksumValid()) {
        line += " OK";
    } else {
        line += QString(" BAD(expected:0x%1)").arg(frame.calcChecksum, 2, 16, QChar('0'));
    }

    // Errors
    if (frame.hasError()) {
        line += "  ERR:";
        if (frame.errorFlags & LINError::PARITY_ERROR)   line += " PARITY";
        if (frame.errorFlags & LINError::CHECKSUM_ERROR)  line += " CHECKSUM";
        if (frame.errorFlags & LINError::NO_RESPONSE)     line += " NO_RESP";
        if (frame.errorFlags & LINError::FRAMING_ERROR)   line += " FRAME";
        if (frame.errorFlags & LINError::TIMEOUT)         line += " TIMEOUT";
    }

    return line;
}

void LINSnifferWindow::addFrame(const LINFrame &frame)
{
    if (mPaused) return;

    mFrames.append(frame);
    if (mFrames.size() > mMaxFrames)
        mFrames.removeFirst();

    mTextDisplay->append(formatFrame(frame));
    mStatusLabel->setText(tr("Frames: %1").arg(mFrames.size()));
}

void LINSnifferWindow::addFrames(const QVector<LINFrame> &frames)
{
    for (const auto &f : frames)
        addFrame(f);
}

void LINSnifferWindow::clear()
{
    mFrames.clear();
    mTextDisplay->clear();
    mStatusLabel->setText(tr("Frames: 0"));
}

void LINSnifferWindow::setShowTimestamps(bool show)
{
    mShowTimestamps = show;
}

void LINSnifferWindow::setShowParity(bool show)
{
    mShowParity = show;
}

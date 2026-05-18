#ifndef LIN_SNIFFER_WINDOW_H
#define LIN_SNIFFER_WINDOW_H

#include <QDialog>
#include <QVector>
#include <QTimer>
#include <QLabel>
#include <QTextEdit>
#include "lin_structs.h"

// ═══════════════════════════════════════════════════════════════════════════
// LINSnifferWindow — real-time LIN frame display similar to CAN sniffer
// ═══════════════════════════════════════════════════════════════════════════

class LINSnifferWindow : public QDialog
{
    Q_OBJECT

public:
    explicit LINSnifferWindow(QWidget *parent = nullptr);
    virtual ~LINSnifferWindow();

public slots:
    void addFrame(const LINFrame &frame);
    void addFrames(const QVector<LINFrame> &frames);
    void clear();
    void setShowTimestamps(bool show);
    void setShowParity(bool show);

private:
    void setupUI();
    QString formatFrame(const LINFrame &frame) const;

    QTextEdit   *mTextDisplay = nullptr;
    QLabel      *mStatusLabel = nullptr;
    QVector<LINFrame> mFrames;
    int          mMaxFrames = 5000;
    bool         mShowTimestamps = true;
    bool         mShowParity = true;
    bool         mPaused = false;
};

#endif // LIN_SNIFFER_WINDOW_H

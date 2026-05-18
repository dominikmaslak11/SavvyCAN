#ifndef J1939WINDOW_H
#define J1939WINDOW_H

#include <QDialog>
#include <QTableWidget>
#include <QLabel>
#include <QVector>
#include "can_structs.h"
#include "bus_protocols/j1939_structs.h"
#include "bus_protocols/j1939_handler.h"

class J1939Window : public QDialog
{
    Q_OBJECT

public:
    explicit J1939Window(QWidget *parent = nullptr);
    ~J1939Window();

public slots:
    void processFrame(const CANFrame &frame);
    void clear();

private:
    void setupUI();
    void onPGNDecoded(uint32_t pgn, QMap<uint32_t, double> values);
    void onDTCUpdated(const QVector<J1939_DTC> &dtcs);
    void onTransportComplete(uint32_t pgn, const QByteArray &data);

    J1939Handler    mHandler;
    QTableWidget   *mPGNTable  = nullptr;
    QTableWidget   *mDTCTable  = nullptr;
    QLabel         *mStatusLabel = nullptr;
    int             mFrameCount = 0;
};

#endif

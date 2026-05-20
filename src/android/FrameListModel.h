#ifndef FRAMELISTMODEL_H
#define FRAMELISTMODEL_H

#include <QAbstractListModel>
#include <QByteArray>
#include "framestore.h"

/// FrameListModel wraps FrameStore as a QAbstractListModel for QML ListView.
///
/// Roles:
///   frameId    — hex string (e.g. "0x7E8")
///   frameData  — hex string (e.g. "01 0C 00 00 ...")
///   dlc        — data length code (int)
///   timestamp  — seconds (double)
///   bus        — bus index (int)
///   extended   — bool
class FrameListModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(bool overwriteMode READ overwriteMode WRITE setOverwriteMode NOTIFY overwriteModeChanged)

public:
    enum Roles {
        FrameIdRole = Qt::UserRole + 1,
        FrameDataRole,
        DlcRole,
        TimestampRole,
        BusRole,
        ExtendedRole,
        ReceivedRole
    };
    Q_ENUM(Roles)

    explicit FrameListModel(FrameStore *store, QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    int count() const;

    Q_INVOKABLE void setOverwriteMode(bool enabled);
    bool overwriteMode() const;

signals:
    void countChanged(int count);
    void overwriteModeChanged(bool enabled);

private slots:
    void onFramesAppended(int count);
    void onFramesReset(int reason);

private:
    FrameStore *mStore;
    bool mOverwriteMode = false;
};

#endif // FRAMELISTMODEL_H

#include "FrameListModel.h"
#include "can_structs.h"

FrameListModel::FrameListModel(FrameStore *store, QObject *parent)
    : QAbstractListModel(parent), mStore(store)
{
    connect(mStore, &FrameStore::framesAppended, this, &FrameListModel::onFramesAppended);
    connect(mStore, &FrameStore::framesReset, this, &FrameListModel::onFramesReset);
}

int FrameListModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) return 0;
    return mStore ? mStore->frameCount() : 0;
}

QVariant FrameListModel::data(const QModelIndex &index, int role) const
{
    if (!mStore || !index.isValid()) return {};

    QVector<CANFrame> frames = mStore->allFrames();
    int row = index.row();
    if (row < 0 || row >= frames.size()) return {};

    const CANFrame &f = frames[row];

    switch (role) {
    case FrameIdRole:
        return QString("0x%1").arg(f.frameId(), 3, 16, QChar('0')).toUpper();
    case FrameDataRole: {
        QByteArray payload = f.payload();
        QStringList bytes;
        for (uint8_t b : payload)
            bytes.append(QString("%1").arg(b, 2, 16, QChar('0')).toUpper());
        return bytes.join(' ');
    }
    case DlcRole:
        return f.payload().size();
    case TimestampRole: {
        auto ts = f.timeStamp();
        return ts.seconds() + ts.microSeconds() / 1e6;
    }
    case BusRole:
        return f.bus;
    case ExtendedRole:
        return f.hasExtendedFrameFormat();
    case ReceivedRole:
        return f.isReceived;
    case Qt::DisplayRole:
        // Summary for simple display
        return QString("[%1] 0x%2  %3")
            .arg(f.bus)
            .arg(f.frameId(), 3, 16, QChar('0'))
            .arg(QString::fromLatin1(f.payload().toHex(' ')));
    default:
        return {};
    }
}

QHash<int, QByteArray> FrameListModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[FrameIdRole]    = "frameId";
    roles[FrameDataRole]  = "frameData";
    roles[DlcRole]        = "dlc";
    roles[TimestampRole]  = "timestamp";
    roles[BusRole]        = "bus";
    roles[ExtendedRole]   = "extended";
    roles[ReceivedRole]   = "isReceived";
    return roles;
}

int FrameListModel::count() const
{
    return mStore ? mStore->frameCount() : 0;
}

void FrameListModel::setOverwriteMode(bool enabled)
{
    if (mOverwriteMode != enabled) {
        mOverwriteMode = enabled;
        emit overwriteModeChanged(enabled);
        if (mStore) mStore->recalcOverwrite();
    }
}

bool FrameListModel::overwriteMode() const
{
    return mOverwriteMode;
}

void FrameListModel::onFramesAppended(int count)
{
    int oldCount = rowCount();
    int newCount = oldCount + count;

    if (oldCount > 0) {
        // For now just reset — efficient enough for moderate frame rates
        // In production we'd use beginInsertRows/endInsertRows
        beginResetModel();
        endResetModel();
    } else {
        beginInsertRows(QModelIndex(), oldCount, newCount - 1);
        endInsertRows();
    }

    emit countChanged(newCount);
}

void FrameListModel::onFramesReset(int /*reason*/)
{
    beginResetModel();
    endResetModel();
    emit countChanged(0);
}

#ifndef FRAMESTORE_H
#define FRAMESTORE_H

#include <QObject>
#include <QVector>
#include <QMap>
#include <QMutex>
#include <QReadWriteLock>
#include "can_structs.h"
#include "connections/canconnection.h"

/// FrameStore is the single source of truth for all CAN frames in SavvyCAN.
///
/// Previously each sub-window received a raw `const QVector<CANFrame>*` and
/// the main window manually synchronized everything.  FrameStore replaces
/// that with a signal-driven architecture:
///
///   - Subscribers connect to signals (framesAppended, framesCleared, …).
///   - The store owns the frame data; external code never holds raw pointers
///     that could dangle.
///   - Thread-safe: all public methods lock internally.
///
/// This is Step 2 of the SavvyCAN modernization plan.
class FrameStore : public QObject
{
    Q_OBJECT

public:
    explicit FrameStore(QObject *parent = nullptr);

    // ── mutation ──────────────────────────────────────────────────────
    void addFrame(const CANFrame &frame);
    void addFrames(const CANConnection *conn, const QVector<CANFrame> &newFrames);
    void clearFrames();
    void normalizeTiming();
    void recalcOverwrite();

    // ── filter operations ──────────────────────────────────────────────
    void setFilterState(uint32_t id, bool enabled);
    void setBusFilterState(uint32_t bus, bool enabled);
    void setAllFilters(bool enabled);
    bool needsFilterRefresh() const;
    void markFiltersRefreshed();

    // ── query (thread-safe snapshots) ──────────────────────────────────
    /// Return a *copy* of the full frame list.  Cheap for small sets;
    /// prefer signals for real-time updates.
    QVector<CANFrame> allFrames() const;

    /// Return a *copy* of the filtered frame list.
    QVector<CANFrame> filteredFrames() const;

    int frameCount() const;
    int filteredFrameCount() const;
    bool isEmpty() const;

    // ── filter snapshot helpers ────────────────────────────────────────
    QMap<int, bool> filterSnapshot() const;
    QMap<int, bool> busFilterSnapshot() const;
    bool anyFiltersActive() const;
    bool anyBusFiltersActive() const;

    // ── index lookup (for centering / timeline sync) ───────────────────
    int indexFromTimeID(uint32_t id, double timestamp) const;

    /// Get the most recent frame for a given ID (used by overwrite mode).
    CANFrame latestFrameForId(uint32_t id) const;

signals:
    /// Emitted when new frames are added.  `count` is how many were added.
    void framesAppended(int count);

    /// Emitted when all frames are cleared (count = -1) or completely
    /// replaced (count = -2, e.g. after loading a new file).
    void framesReset(int reason);

    /// Emitted periodically for bulk updates (bundled refresh).
    void framesUpdated(int count);

    /// Emitted when the filter list changes.
    void filtersChanged();

private:
    mutable QReadWriteLock mLock;

    QVector<CANFrame> mAllFrames;
    QVector<CANFrame> mFilteredFrames;
    QMap<int, bool>   mFilters;
    QMap<int, bool>   mBusFilters;
    bool              mNeedFilterRefresh = false;

    // ── internal helpers ───────────────────────────────────────────────
    bool passesFilter(const CANFrame &frame) const;
    void rebuildFilteredList();
    void addFrameInternal(const CANFrame &frame);
};

#endif // FRAMESTORE_H

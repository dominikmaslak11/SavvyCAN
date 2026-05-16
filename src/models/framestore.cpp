#include "framestore.h"
#include <QDateTime>
#include <QDebug>

FrameStore::FrameStore(QObject *parent)
    : QObject(parent)
{
}

// ── mutation ──────────────────────────────────────────────────────────

void FrameStore::addFrame(const CANFrame &frame)
{
    QWriteLocker lock(&mLock);
    addFrameInternal(frame);
    lock.unlock();
    emit framesAppended(1);
}

void FrameStore::addFrames(const CANConnection *, const QVector<CANFrame> &newFrames)
{
    if (newFrames.isEmpty()) return;

    QWriteLocker lock(&mLock);
    for (const auto &f : newFrames)
        addFrameInternal(f);
    lock.unlock();

    emit framesAppended(newFrames.size());
}

void FrameStore::addFrameInternal(const CANFrame &frame)
{
    mAllFrames.append(frame);

    if (passesFilter(frame))
        mFilteredFrames.append(frame);
}

void FrameStore::clearFrames()
{
    QWriteLocker lock(&mLock);
    mAllFrames.clear();
    mAllFrames.squeeze();
    mFilteredFrames.clear();
    mFilteredFrames.squeeze();
    lock.unlock();

    emit framesReset(-1);
}

void FrameStore::normalizeTiming()
{
    QWriteLocker lock(&mLock);
    if (mAllFrames.isEmpty()) return;

    int64_t firstTime = mAllFrames.first().timeStamp().microSeconds();
    for (auto &f : mAllFrames) {
        f.setTimeStamp(QCanBusFrame::TimeStamp(0,
            f.timeStamp().microSeconds() - firstTime));
    }
    // rebuild filtered list since timestamps changed
    rebuildFilteredList();
    lock.unlock();

    emit framesReset(-2); // "all frames replaced"
}

void FrameStore::recalcOverwrite()
{
    // Deduplicate: keep only the latest frame per (ID, bus) pair,
    // computing frameCount and timedelta like CANFrameModel does.
    if (mAllFrames.isEmpty()) return;

    QWriteLocker lock(&mLock);

    // key = (id << 5) | bus — safe since bus < 32 and id < 2^27
    QHash<uint64_t, CANFrame> seen;
    QVector<CANFrame> deduped;
    deduped.reserve(mAllFrames.size() / 4);

    for (auto &f : mAllFrames) {
        if (f.frameType() != QCanBusFrame::DataFrame) continue;
        uint64_t key = (static_cast<uint64_t>(f.frameId()) << 5) | static_cast<uint64_t>(f.bus & 0x1F);
        auto it = seen.find(key);
        if (it == seen.end()) {
            f.timedelta = 0;
            f.frameCount = 1;
            seen.insert(key, f);
        } else {
            f.timedelta = f.timeStamp().microSeconds() - it->timeStamp().microSeconds();
            f.frameCount = it->frameCount + 1;
            *it = f;
        }
    }

    deduped = seen.values().toVector();

    mAllFrames = deduped;
    mAllFrames.squeeze();
    rebuildFilteredList();

    lock.unlock();

    emit framesReset(-3); // "overwrite mode applied"
}

// ── filters ───────────────────────────────────────────────────────────

void FrameStore::setFilterState(uint32_t id, bool enabled)
{
    QWriteLocker lock(&mLock);
    mFilters[static_cast<int>(id)] = enabled;
    rebuildFilteredList();
    mNeedFilterRefresh = true;
    lock.unlock();

    emit filtersChanged();
}

void FrameStore::setBusFilterState(uint32_t bus, bool enabled)
{
    QWriteLocker lock(&mLock);
    mBusFilters[static_cast<int>(bus)] = enabled;
    rebuildFilteredList();
    mNeedFilterRefresh = true;
    lock.unlock();

    emit filtersChanged();
}

void FrameStore::setAllFilters(bool enabled)
{
    QWriteLocker lock(&mLock);
    for (auto it = mFilters.begin(); it != mFilters.end(); ++it)
        it.value() = enabled;
    rebuildFilteredList();
    mNeedFilterRefresh = true;
    lock.unlock();

    emit filtersChanged();
}

bool FrameStore::needsFilterRefresh() const
{
    QReadLocker lock(&mLock);
    return mNeedFilterRefresh;
}

void FrameStore::markFiltersRefreshed()
{
    QWriteLocker lock(&mLock);
    mNeedFilterRefresh = false;
}

bool FrameStore::passesFilter(const CANFrame &frame) const
{
    // If no filters active at all, everything passes
    if (!anyFiltersActive() && !anyBusFiltersActive())
        return true;

    // Check bus filter
    if (anyBusFiltersActive()) {
        auto it = mBusFilters.find(frame.bus);
        if (it != mBusFilters.end() && !it.value())
            return false; // bus is explicitly disabled
    }

    // Check ID filter
    if (anyFiltersActive()) {
        auto it = mFilters.find(static_cast<int>(frame.frameId()));
        if (it != mFilters.end() && !it.value())
            return false;
        // If the ID isn't in the filter map, it passes by default
    }

    return true;
}

void FrameStore::rebuildFilteredList()
{
    mFilteredFrames.clear();
    mFilteredFrames.reserve(mAllFrames.size());

    if (!anyFiltersActive() && !anyBusFiltersActive()) {
        mFilteredFrames = mAllFrames;
        return;
    }

    for (const auto &f : mAllFrames) {
        if (passesFilter(f))
            mFilteredFrames.append(f);
    }
}

bool FrameStore::anyFiltersActive() const
{
    // A filter is "active" if ANY entry in the map has a false value
    // (meaning that ID is being filtered OUT).
    // But actually the semantics are: filters are IDs that we WANT to see.
    // Let me look at how the original code works...

    // In the original CANFrameModel, filters work like this:
    // - filter map is populated with all unique IDs in the frame list
    // - default value for a key is true (checked = show)
    // - to filter OUT an ID, set it to false (unchecked)
    // So a filter is "active" when at least one entry exists.
    // But for efficiency, we check if any entry is false.

    return !mFilters.isEmpty() || !mBusFilters.isEmpty();
}

bool FrameStore::anyBusFiltersActive() const
{
    return !mBusFilters.isEmpty();
}

// ── query ─────────────────────────────────────────────────────────────

QVector<CANFrame> FrameStore::allFrames() const
{
    QReadLocker lock(&mLock);
    return mAllFrames;
}

QVector<CANFrame> FrameStore::filteredFrames() const
{
    QReadLocker lock(&mLock);
    return mFilteredFrames;
}

int FrameStore::frameCount() const
{
    QReadLocker lock(&mLock);
    return mAllFrames.size();
}

int FrameStore::filteredFrameCount() const
{
    QReadLocker lock(&mLock);
    return mFilteredFrames.size();
}

bool FrameStore::isEmpty() const
{
    QReadLocker lock(&mLock);
    return mAllFrames.isEmpty();
}

QMap<int, bool> FrameStore::filterSnapshot() const
{
    QReadLocker lock(&mLock);
    return mFilters;
}

QMap<int, bool> FrameStore::busFilterSnapshot() const
{
    QReadLocker lock(&mLock);
    return mBusFilters;
}

int FrameStore::indexFromTimeID(uint32_t id, double timestamp) const
{
    QReadLocker lock(&mLock);

    // Convert seconds-based timestamp to microseconds
    int64_t targetUs = static_cast<int64_t>(timestamp * 1000000.0);

    // Simple linear scan — will be replaced with index later
    int bestIdx = -1;
    int64_t bestDiff = std::numeric_limits<int64_t>::max();

    for (int i = 0; i < mAllFrames.size(); ++i) {
        if (mAllFrames[i].frameId() != id) continue;
        int64_t diff = qAbs(mAllFrames[i].timeStamp().microSeconds() - targetUs);
        if (diff < bestDiff) {
            bestDiff = diff;
            bestIdx = i;
        }
    }

    return bestIdx;
}

CANFrame FrameStore::latestFrameForId(uint32_t id) const
{
    QReadLocker lock(&mLock);

    CANFrame result;
    result.setFrameId(id);
    bool found = false;

    for (const auto &f : mAllFrames) {
        if (f.frameId() == id) {
            result = f;
            found = true;
            // Continue — we want the LAST one
        }
    }

    if (!found)
        result.setTimeStamp(QCanBusFrame::TimeStamp(0, 0));

    return result;
}

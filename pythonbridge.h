#ifndef PYTHONBRIDGE_H
#define PYTHONBRIDGE_H

// Qt headers MUST come before pybind11 because Qt's `slots` macro
// conflicts with Python's `PyType_Spec.slots` member.
// We undef `slots` temporarily, include pybind11, then restore.
#include <QObject>
#include <QString>
#include <QVector>

#pragma push_macro("slots")
#undef slots
#include <pybind11/embed.h>
#include <memory>
#pragma pop_macro("slots")

#include "can_structs.h"

class FrameStore;

/// PythonBridge embeds a CPython interpreter and exposes the SavvyCAN
/// frame API to Python scripts.  Replaces the legacy QJSEngine scripting.
///
/// Usage from Python:
///   import savvycan
///   frames = savvycan.get_frames(limit=100, filter_id=0x7E0)
///   savvycan.send_frame(id=0x7E0, data=[0x02, 0x10, 0x01], bus=0)
///   dbc = savvycan.get_dbc_signals(0x7E0)
class PythonBridge : public QObject
{
    Q_OBJECT

public:
    explicit PythonBridge(FrameStore *store, QObject *parent = nullptr);
    ~PythonBridge();

    /// Execute a Python script (multi-line string).
    QString execute(const QString &code);

    /// Execute a single Python expression, returning its repr.
    QString evaluate(const QString &expr);

    /// Reset the Python interpreter state.
    void reset();

    /// Whether the interpreter is healthy.
    bool isReady() const;

signals:
    void outputReceived(const QString &text);
    void errorReceived(const QString &text);

private:
    void setupModule();

    std::unique_ptr<pybind11::scoped_interpreter> mGuard;
    FrameStore *mStore;
    bool mReady = false;
};

#endif // PYTHONBRIDGE_H

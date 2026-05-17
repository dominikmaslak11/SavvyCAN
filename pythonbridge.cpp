#include "pythonbridge.h"
#include "framestore.h"
#include "dbc/dbchandler.h"

#pragma push_macro("slots")
#undef slots
#include <pybind11/stl.h>
#pragma pop_macro("slots")
#include <QDebug>
#include <sstream>

namespace py = pybind11;

PythonBridge::PythonBridge(FrameStore *store, QObject *parent)
    : QObject(parent), mStore(store)
{
    // PyImport_AppendInittab MUST be called BEFORE Py_Initialize().
    // Register the savvycan module before creating the interpreter.
    try {
        static bool s_savvycanRegistered = false;
        if (!s_savvycanRegistered) {
            PyImport_AppendInittab("savvycan", []() -> PyObject* {
                auto m = py::module_::create_extension_module(
                    "savvycan", "SavvyCAN automation API",
                    new py::module_::module_def{});
                return m.ptr();
            });
            s_savvycanRegistered = true;
        }
    } catch (...) {}

    // Create the Python interpreter AFTER registering the module
    try {
        mGuard = std::make_unique<pybind11::scoped_interpreter>();
        setupModule();
        mReady = true;
    } catch (const std::exception &e) {
        qWarning() << "PythonBridge init failed:" << e.what();
        mReady = false;
        // If Py_Initialize() succeeded but the interpreter setup threw, finalize
        // Python so we don't crash during process teardown.
        if (Py_IsInitialized())
            Py_Finalize();
    } catch (...) {
        qWarning() << "PythonBridge init failed with unknown exception";
        mReady = false;
        if (Py_IsInitialized())
            Py_Finalize();
    }

    if (mReady) {
        // Redirect Python stdout/stderr to Qt signals
        py::module_ sys = py::module_::import("sys");
        sys.attr("stdout") = py::cpp_function([this](const std::string &text) {
            emit outputReceived(QString::fromStdString(text));
        });
        sys.attr("stderr") = py::cpp_function([this](const std::string &text) {
            emit errorReceived(QString::fromStdString(text));
        });
    }
}

PythonBridge::~PythonBridge() = default;

void PythonBridge::setupModule()
{
    // ── savvycan Python module ──────────────────────────────────────
    py::module_ m = py::module_::create_extension_module(
        "savvycan", "SavvyCAN automation API",
        new py::module_::module_def{}
    );

    // savvycan.get_frames(limit=100, filter_id=None)
    m.def("get_frames", [this](int limit, py::object filter_id) -> py::list {
        auto frames = mStore->allFrames();
        py::list result;
        int count = 0;

        for (int i = frames.size() - 1; i >= 0 && count < limit; --i) {
            const auto &f = frames[i];
            if (!filter_id.is_none()) {
                uint32_t fid = filter_id.cast<uint32_t>();
                if (f.frameId() != fid) continue;
            }
            py::dict d;
            d["id"] = f.frameId();
            d["extended"] = f.hasExtendedFrameFormat();
            d["bus"] = f.bus;
            d["timestamp_us"] = f.timeStamp().microSeconds();
            d["is_rx"] = f.isReceived;

            const auto &payload = f.payload();
            py::list data;
            for (int j = 0; j < payload.size(); ++j)
                data.append(static_cast<uint8_t>(payload[j]));
            d["data"] = data;
            d["dlc"] = payload.size();

            result.append(d);
            ++count;
        }
        return result;
    }, py::arg("limit") = 100, py::arg("filter_id") = py::none());

    // savvycan.frame_count()
    m.def("frame_count", [this]() {
        return mStore->frameCount();
    });

    // savvycan.send_frame(id, data, bus=0)
    m.def("send_frame", [this](uint32_t id, py::list data, int bus) {
        CANFrame frame;
        frame.setFrameId(id);
        if (id > 0x7FF) frame.setExtendedFrameFormat(true);
        frame.bus = bus;
        frame.isReceived = false;
        frame.setFrameType(QCanBusFrame::DataFrame);

        QByteArray payload;
        for (auto item : data)
            payload.append(static_cast<char>(item.cast<int>()));
        frame.setPayload(payload);

        // Frame will be picked up by CANConManager for sending
        mStore->addFrame(frame);
        return true;
    }, py::arg("id"), py::arg("data"), py::arg("bus") = 0);

    // savvycan.clear_frames()
    m.def("clear_frames", [this]() {
        mStore->clearFrames();
    });

    // savvycan.get_dbc_signals(frame_id) → list of dicts
    m.def("get_dbc_signals", [this](uint32_t id) -> py::list {
        py::list result;
        auto *dbc = DBCHandler::getReference();
        if (!dbc) return result;

        CANFrame frame;
        frame.setFrameId(id);
        DBC_MESSAGE *msg = dbc->findMessage(frame);
        if (!msg) return result;

        for (int j = 0; j < msg->sigHandler->getCount(); ++j) {
            DBC_SIGNAL *sig = msg->sigHandler->findSignalByIdx(j);
            if (!sig) continue;

            py::dict d;
            d["name"] = sig->name.toStdString();
            d["start_bit"] = sig->startBit;
            d["size"] = sig->signalSize;
            d["factor"] = sig->factor;
            d["bias"] = sig->bias;
            d["signed"] = (sig->valType == SIGNED_INT);
            d["little_endian"] = sig->intelByteOrder;
            d["unit"] = sig->unitName.toStdString();
            d["comment"] = sig->comment.toStdString();
            result.append(d);
        }
        return result;
    }, py::arg("frame_id"));

    // Module registration moved to constructor body (before Py_Initialize)
    // Pre-load signals so Python can reference them
    py::globals()["savvycan"] = m;
}

QString PythonBridge::execute(const QString &code)
{
    if (!mReady) return "PythonBridge not initialized";

    try {
        py::exec(code.toStdString());
        return {};
    } catch (const py::error_already_set &e) {
        QString err = QString::fromStdString(e.what());
        emit errorReceived(err);
        return err;
    } catch (const std::exception &e) {
        QString err = QString::fromStdString(e.what());
        emit errorReceived(err);
        return err;
    }
}

QString PythonBridge::evaluate(const QString &expr)
{
    if (!mReady) return "PythonBridge not initialized";

    try {
        py::object result = py::eval(expr.toStdString());
        return QString::fromStdString(py::repr(result));
    } catch (const py::error_already_set &e) {
        QString err = QString::fromStdString(e.what());
        emit errorReceived(err);
        return QString("Error: %1").arg(err);
    }
}

void PythonBridge::reset()
{
    // scoped_interpreter is RAII — it can't be reset in-place.
    // The PythonBridge must be destroyed and recreated.
    qWarning() << "PythonBridge::reset() — recreate the bridge instead";
}

bool PythonBridge::isReady() const
{
    return mReady;
}

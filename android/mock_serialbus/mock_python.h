#ifndef MOCK_PYTHON_H
#define MOCK_PYTHON_H

#include <QObject>
#include <QString>

// Minimal stubs for Python/pybind11 (not available on Android)

// Stub pybind11::scoped_interpreter and related
namespace pybind11 {
    class scoped_interpreter {
    public:
        scoped_interpreter() {}
        ~scoped_interpreter() {}
    };
}

class PythonBridgePrivate
{
public:
    PythonBridgePrivate() {}
    ~PythonBridgePrivate() {}
    bool init() { return false; }
};

class PythonConsolePrivate
{
public:
    PythonConsolePrivate() {}
    ~PythonConsolePrivate() {}
};

#endif // MOCK_PYTHON_H

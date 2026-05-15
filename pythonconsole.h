#ifndef PYTHONCONSOLE_H
#define PYTHONCONSOLE_H

#include <QWidget>
#include <QTextEdit>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>

class PythonBridge;

/// Interactive Python REPL console widget.
/// Connects to PythonBridge for code execution and output display.
class PythonConsole : public QWidget
{
    Q_OBJECT

public:
    explicit PythonConsole(PythonBridge *bridge, QWidget *parent = nullptr);

    /// Append output text to the console.
    void appendOutput(const QString &text);

    /// Append error text to the console.
    void appendError(const QString &text);

private slots:
    void executeLine();

private:
    void setupUi();

    PythonBridge *mBridge;
    QTextEdit    *mOutput;
    QLineEdit    *mInput;
    QPushButton  *mRunBtn;
};

#endif // PYTHONCONSOLE_H

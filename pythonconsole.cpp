#include "pythonconsole.h"
#include "pythonbridge.h"
#include <QKeyEvent>

PythonConsole::PythonConsole(PythonBridge *bridge, QWidget *parent)
    : QWidget(parent), mBridge(bridge)
{
    setupUi();

    if (mBridge) {
        connect(mBridge, &PythonBridge::outputReceived, this, &PythonConsole::appendOutput);
        connect(mBridge, &PythonBridge::errorReceived, this, &PythonConsole::appendError);
    }
}

void PythonConsole::setupUi()
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(4, 4, 4, 4);
    layout->setSpacing(4);

    auto *header = new QLabel("Python REPL", this);
    header->setStyleSheet("color: #00e5ff; font-weight: bold; font-size: 13px;");
    layout->addWidget(header);

    mOutput = new QTextEdit(this);
    mOutput->setReadOnly(true);
    mOutput->setStyleSheet(R"(
        QTextEdit {
            background-color: #0a0a1a;
            color: #e0e0f0;
            border: 1px solid #1e1e3a;
            border-radius: 4px;
            font-family: "Consolas", "Courier New", monospace;
            font-size: 12px;
        }
    )");
    mOutput->setMaximumHeight(200);
    layout->addWidget(mOutput);

    auto *inputLayout = new QHBoxLayout();
    mInput = new QLineEdit(this);
    mInput->setPlaceholderText(">>> savvycan.send_frame(...)");
    mInput->setStyleSheet(R"(
        QLineEdit {
            background-color: #12122a;
            color: #00e5ff;
            border: 1px solid #1e1e3a;
            border-radius: 4px;
            padding: 6px;
            font-family: "Consolas", "Courier New", monospace;
            font-size: 12px;
        }
        QLineEdit:focus { border-color: #00e5ff; }
    )");
    connect(mInput, &QLineEdit::returnPressed, this, &PythonConsole::executeLine);
    inputLayout->addWidget(mInput);

    mRunBtn = new QPushButton("Run", this);
    mRunBtn->setCursor(Qt::PointingHandCursor);
    mRunBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #1a1a35;
            color: #00e5ff;
            border: 1px solid #1e1e3a;
            border-radius: 4px;
            padding: 6px 14px;
            font-weight: bold;
        }
        QPushButton:hover { background-color: #222244; }
    )");
    connect(mRunBtn, &QPushButton::clicked, this, &PythonConsole::executeLine);
    inputLayout->addWidget(mRunBtn);

    layout->addLayout(inputLayout);

    appendOutput("Python 3.14 ready. Try: savvycan.frame_count()\n");
}

void PythonConsole::executeLine()
{
    QString code = mInput->text().trimmed();
    if (code.isEmpty()) return;

    mInput->clear();
    appendOutput(QString(">>> %1\n").arg(code));

    if (!mBridge || !mBridge->isReady()) {
        appendError("PythonBridge not available\n");
        return;
    }

    // If it looks like an expression (no newlines, doesn't start with for/if/def/import/class), evaluate it
    bool isExpression = !code.contains('\n')
        && !code.startsWith("for ")
        && !code.startsWith("if ")
        && !code.startsWith("def ")
        && !code.startsWith("import ")
        && !code.startsWith("from ")
        && !code.startsWith("class ")
        && !code.startsWith("while ")
        && !code.startsWith("try:")
        && !code.startsWith("with ");

    if (isExpression) {
        QString result = mBridge->evaluate(code);
        if (!result.isEmpty() && !result.startsWith("Error:"))
            appendOutput(result + "\n");
    } else {
        QString err = mBridge->execute(code);
        if (!err.isEmpty())
            appendError(err + "\n");
    }
}

void PythonConsole::appendOutput(const QString &text)
{
    mOutput->moveCursor(QTextCursor::End);
    mOutput->setTextColor(QColor("#e0e0f0"));
    mOutput->insertPlainText(text);
    mOutput->moveCursor(QTextCursor::End);
}

void PythonConsole::appendError(const QString &text)
{
    mOutput->moveCursor(QTextCursor::End);
    mOutput->setTextColor(QColor("#ff4081"));
    mOutput->insertPlainText(text);
    mOutput->moveCursor(QTextCursor::End);
}

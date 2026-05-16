#include "sidebarwidget.h"
#include "config.h"
#include "pythonconsole.h"
#include "framestore.h"
#include <QApplication>
#include <QKeyEvent>
#include <QFrame>
#include <QScrollArea>
#include <QTimer>
#include <algorithm>

SidebarWidget::SidebarWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
    buildCommandList();
    applyStyles();

    setMinimumWidth(200);
    setMaximumWidth(280);
}

void SidebarWidget::setupUi()
{
    mMainLayout = new QVBoxLayout(this);
    mMainLayout->setContentsMargins(8, 8, 8, 8);
    mMainLayout->setSpacing(6);

    // ── Category Panel ─────────────────────────────────────────────
    mCategoryPanel = new QWidget(this);
    auto *catLayout = new QVBoxLayout(mCategoryPanel);
    catLayout->setContentsMargins(0, 0, 0, 0);
    catLayout->setSpacing(4);

    // Logo / title
    auto *titleLabel = new QLabel("SAVVYCAN", mCategoryPanel);
    titleLabel->setObjectName("sidebarTitle");
    titleLabel->setAlignment(Qt::AlignCenter);
    catLayout->addWidget(titleLabel);

    auto *verLabel = new QLabel(QString::number(VERSION), mCategoryPanel);
    verLabel->setObjectName("sidebarVersion");
    verLabel->setAlignment(Qt::AlignCenter);
    catLayout->addWidget(verLabel);

    // Separator
    auto *sep1 = new QFrame(mCategoryPanel);
    sep1->setFrameShape(QFrame::HLine);
    sep1->setObjectName("sidebarSeparator");
    catLayout->addWidget(sep1);

    // Category buttons
    struct Category { QString id; QString label; QString icon; };
    const QVector<Category> cats = {
        {"dashboard",  "Dashboard",    "◉"},
        {"re_tools",   "RE Tools",     "◎"},
        {"send",       "Send Frames",  "▶"},
        {"dbc",        "DBC Manager",  "◈"},
        {"connection", "Connection",   "⬡"},
        {"settings",   "Settings",     "⚙"},
    };

    for (const auto &cat : cats) {
        auto *btn = new QPushButton(QString("%1  %2").arg(cat.icon, cat.label), mCategoryPanel);
        btn->setObjectName("sidebarCategoryBtn");
        btn->setCursor(Qt::PointingHandCursor);
        connect(btn, &QPushButton::clicked, this, [this, id = cat.id] {
            onCategoryClicked(id);
        });
        catLayout->addWidget(btn);
    }

    catLayout->addStretch();

    // Ctrl+K hint
    auto *hintLabel = new QLabel("Ctrl+K  Command Palette", mCategoryPanel);
    hintLabel->setObjectName("sidebarHint");
    hintLabel->setAlignment(Qt::AlignCenter);
    catLayout->addWidget(hintLabel);

    mMainLayout->addWidget(mCategoryPanel);

    // ── Command Palette Overlay ─────────────────────────────────────
    mCommandOverlay = new QWidget(this);
    mCommandOverlay->setObjectName("commandOverlay");
    mCommandOverlay->setVisible(false);
    auto *overlayLayout = new QVBoxLayout(mCommandOverlay);
    overlayLayout->setContentsMargins(12, 12, 12, 12);
    overlayLayout->setSpacing(8);

    mSearchBox = new QLineEdit(mCommandOverlay);
    mSearchBox->setObjectName("commandSearch");
    mSearchBox->setPlaceholderText("Search tools...");
    mSearchBox->setClearButtonEnabled(true);
    connect(mSearchBox, &QLineEdit::textChanged, this, &SidebarWidget::setFilterText);
    overlayLayout->addWidget(mSearchBox);

    mCommandList = new QListWidget(mCommandOverlay);
    mCommandList->setObjectName("commandList");
    mCommandList->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    connect(mCommandList, &QListWidget::itemClicked, this, &SidebarWidget::onCommandSelected);
    overlayLayout->addWidget(mCommandList);

    mMainLayout->addWidget(mCommandOverlay);
}

void SidebarWidget::buildCommandList()
{
    mAllTools = {
        {"graph",         "New Graphing Window",         "re_tools"},
        {"flowview",      "Flow View",                   "re_tools"},
        {"frameinfo",     "Frame Data Analysis",         "re_tools"},
        {"comparator",    "File Comparison",             "re_tools"},
        {"dbccompare",    "DBC Comparison",              "re_tools"},
        {"range",         "Range State",                 "re_tools"},
        {"discrete",      "Single/Multi State",          "re_tools"},
        {"isotp",         "ISO-TP Decoder",              "re_tools"},
        {"sniffer",       "Sniffer",                     "re_tools"},
        {"bisect",        "Capture Bisector",            "re_tools"},
        {"signalviewer",  "Signal Viewer",               "re_tools"},
        {"temporal",      "Temporal Graph",              "re_tools"},
        {"playback",      "Frame Playback",              "send"},
        {"framesender",   "Custom Frame Sender",         "send"},
        {"scripting",     "Scripting Interface",         "send"},
        {"fuzzing",       "Fuzzing",                     "send"},
        {"udsscan",       "UDS Scanner",                 "send"},
        {"udsfirmware",   "UDS Firmware Update",         "send"},
        {"motorctrl",     "Motor Control Config",        "send"},
        {"canbridge",     "CAN Bridge",                  "send"},
        {"dbcfile",       "DBC File Manager",            "dbc"},
        {"connection",    "Connection Window",           "connection"},
        {"settings",      "Preferences",                 "settings"},
        {"loadfile",      "Load Log File",               "dashboard"},
        {"savefile",      "Save Log File",               "dashboard"},
        {"clearframes",   "Clear Frames",                "dashboard"},
    };

    // Sort alphabetically
    std::sort(mAllTools.begin(), mAllTools.end(), [](const ToolEntry &a, const ToolEntry &b) {
        return a.name < b.name;
    });
}

void SidebarWidget::setFilterText(const QString &text)
{
    mCommandList->clear();

    for (const auto &tool : mAllTools) {
        if (text.isEmpty() ||
            tool.name.contains(text, Qt::CaseInsensitive) ||
            tool.category.contains(text, Qt::CaseInsensitive))
        {
            auto *item = new QListWidgetItem(
                QString("%1  [%2]").arg(tool.name, tool.category));
            item->setData(Qt::UserRole, tool.id);
            mCommandList->addItem(item);
        }
    }

    // Select first item
    if (mCommandList->count() > 0)
        mCommandList->setCurrentRow(0);
}

void SidebarWidget::toggleCommandPalette()
{
    bool visible = !mCommandOverlay->isVisible();
    mCategoryPanel->setVisible(!visible);
    mCommandOverlay->setVisible(visible);

    if (visible) {
        mSearchBox->clear();
        mSearchBox->setFocus();
        setFilterText("");
    }
}

void SidebarWidget::onCategoryClicked(const QString &category)
{
    if (category == "dashboard")
        emit toolSelected("clearframes"); // placeholder — dashboard shows main view
    else
        emit toolSelected(category);
}

void SidebarWidget::onCommandSelected(QListWidgetItem *item)
{
    if (!item) return;
    QString toolId = item->data(Qt::UserRole).toString();
    mCommandOverlay->setVisible(false);
    mCategoryPanel->setVisible(true);
    emit toolSelected(toolId);
}

void SidebarWidget::setPythonConsole(PythonConsole *console)
{
    if (mPythonConsole) {
        mMainLayout->removeWidget(mPythonConsole);
    }
    mPythonConsole = console;
    if (console) {
        mMainLayout->addWidget(console);
    }
}

void SidebarWidget::toggleTheme()
{
    mDarkTheme = !mDarkTheme;
    if (mDarkTheme) {
        applyStyles();
    } else {
        // Light theme — simplified, just swap background colors
        setStyleSheet(QStringLiteral(R"(
            SidebarWidget {
                background-color: #f0f0f5;
                border-right: 1px solid #ccc;
            }
            QLabel#sidebarTitle { color: #222; font-size: 18px; font-weight: bold; }
            QLabel#sidebarVersion { color: #888; }
            QLabel#sidebarHint { color: #aaa; }
            QPushButton#sidebarCategoryBtn {
                background: transparent; color: #444; border: 1px solid transparent;
                border-radius: 6px; padding: 10px 14px; font-size: 13px; text-align: left;
            }
            QPushButton#sidebarCategoryBtn:hover { background: #e0e0e8; color: #222; }
            QWidget#commandOverlay { background-color: #f0f0f5; }
            QLineEdit#commandSearch { background: #fff; color: #222; border: 1px solid #ccc;
                border-radius: 8px; padding: 12px 16px; font-size: 15px; }
            QLineEdit#commandSearch:focus { border-color: #555; }
            QListWidget#commandList { background: transparent; border: none; color: #444; }
            QListWidget#commandList::item:selected { background: #d0d0e0; color: #222; }
        )"));
    }
}

void SidebarWidget::setFrameStore(FrameStore *store)
{
    mStore = store;
    // Live-stats widgets are not yet created in setupUi();
    // signal connections will be wired when the stats panel is added.
}

void SidebarWidget::onQuickSend()
{
    if (!mStore) return;

    bool ok;
    QString idText = mQuickSendId->text().trimmed();
    uint32_t id = idText.toUInt(&ok, 0);  // auto-detect hex
    if (!ok) {
        mQuickSendId->setStyleSheet("QLineEdit { border: 1px solid #ff4444; }");
        return;
    }
    mQuickSendId->setStyleSheet(QString());

    QString dataText = mQuickSendData->text().simplified();
    QStringList hexBytes = dataText.split(' ', Qt::SkipEmptyParts);
    QByteArray payload;
    for (const QString &hb : hexBytes) {
        uint32_t b = hb.toUInt(&ok, 16);
        if (!ok || b > 255) {
            mQuickSendData->setStyleSheet("QLineEdit { border: 1px solid #ff4444; }");
            return;
        }
        payload.append(static_cast<char>(b));
    }
    mQuickSendData->setStyleSheet(QString());

    CANFrame frame;
    frame.setFrameId(id);
    if (id > 0x7FF) frame.setExtendedFrameFormat(true);
    frame.isReceived = false;
    frame.setFrameType(QCanBusFrame::DataFrame);
    frame.setPayload(payload);

    mStore->addFrame(frame);
}

void SidebarWidget::applyStyles()
{
    setStyleSheet(QStringLiteral(R"(
        SidebarWidget {
            background-color: #0d0d22;
            border-right: 1px solid #1e1e3a;
        }

        QLabel#sidebarTitle {
            color: #00e5ff;
            font-size: 18px;
            font-weight: bold;
            letter-spacing: 4px;
            margin-top: 12px;
        }

        QLabel#sidebarVersion {
            color: #555566;
            font-size: 11px;
        }

        QFrame#sidebarSeparator {
            color: #1e1e3a;
            margin: 8px 0px;
        }

        QLabel#sidebarHint {
            color: #444466;
            font-size: 10px;
            margin-bottom: 8px;
        }

        QPushButton#sidebarCategoryBtn {
            background-color: transparent;
            color: #8899aa;
            border: 1px solid transparent;
            border-radius: 6px;
            padding: 10px 14px;
            font-size: 13px;
            text-align: left;
            font-weight: 500;
        }

        QPushButton#sidebarCategoryBtn:hover {
            background-color: rgba(0, 229, 255, 0.08);
            border-color: #1e1e3a;
            color: #00e5ff;
        }

        QPushButton#sidebarCategoryBtn:pressed {
            background-color: rgba(0, 229, 255, 0.15);
        }

        QWidget#commandOverlay {
            background-color: #0d0d22;
        }

        QLineEdit#commandSearch {
            background-color: #1a1a35;
            color: #e0e0f0;
            border: 1px solid #1e1e3a;
            border-radius: 8px;
            padding: 12px 16px;
            font-size: 15px;
            selection-background-color: #00e5ff;
            selection-color: #000000;
        }

        QLineEdit#commandSearch:focus {
            border-color: #00e5ff;
        }

        QListWidget#commandList {
            background-color: transparent;
            border: none;
            color: #8899aa;
            font-size: 13px;
            outline: none;
        }

        QListWidget#commandList::item {
            padding: 10px 14px;
            border-radius: 4px;
        }

        QListWidget#commandList::item:selected {
            background-color: rgba(0, 229, 255, 0.12);
            color: #00e5ff;
        }
    )"));
}

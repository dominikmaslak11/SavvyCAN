#ifndef SIDEBARWIDGET_H
#define SIDEBARWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QListWidget>
#include <QLabel>
#include <QStackedWidget>

class PythonConsole;
class PythonBridge;
class FrameStore;

/// Futuristic sidebar navigation replacing the traditional menu bar.
///
/// Provides:
///   - Category buttons (Dashboard, RE Tools, Send, DBC, Settings)
///   - Command palette (Ctrl+K) for quick tool access
///   - Collapsible design for small screens
class SidebarWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SidebarWidget(QWidget *parent = nullptr);

    /// Set the command palette filter text.
    void setFilterText(const QString &text);

    /// Show/hide the command palette overlay.
    void toggleCommandPalette();

    /// Set the Python console widget (injected by MainWindow).
    void setPythonConsole(PythonConsole *console);
    void setFrameStore(FrameStore *store);

    /// Toggle dark/light theme.
    void toggleTheme();

    /// Connect to FrameStore for live stats updates.
    void connectFrameStore(FrameStore *store);

    /// Update the live stats labels.
    void refreshStats();

signals:
    /// Emitted when the user selects a tool category.
    void toolSelected(const QString &toolId);

    /// Emitted when the user wants to close the command palette.
    void commandPaletteDismissed();

private slots:
    void onCategoryClicked(const QString &category);
    void onCommandSelected(QListWidgetItem *item);
    void onQuickSend();

private:
    void setupUi();
    void buildCommandList();
    void applyStyles();

    QVBoxLayout  *mMainLayout;
    QLineEdit    *mSearchBox;
    QListWidget  *mCommandList;
    QWidget      *mCommandOverlay;
    QWidget      *mCategoryPanel;
    PythonConsole *mPythonConsole = nullptr;
    bool           mDarkTheme = true;

    // Live stats
    QLabel       *mStatsFrames;
    QLabel       *mStatsBuses;
    FrameStore   *mStore = nullptr;

    // Quick Send
    QLineEdit    *mQuickSendId;
    QLineEdit    *mQuickSendData;
    QPushButton  *mQuickSendBtn;

    struct ToolEntry {
        QString id;
        QString name;
        QString category;
    };
    QVector<ToolEntry> mAllTools;
};

#endif // SIDEBARWIDGET_H

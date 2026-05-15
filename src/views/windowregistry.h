#ifndef WINDOWREGISTRY_H
#define WINDOWREGISTRY_H

#include <QObject>
#include <QHash>
#include <QDialog>
#include <functional>

/// WindowRegistry replaces the old pattern of storing 20+ raw window pointers
/// in MainWindow together with the `killEmAll()` cleanup method.
///
/// Windows are created lazily (on first `show()`) and cached.  Graph windows
/// support multiple independent instances; all other windows are singletons.
///
/// Usage:
///   registry.show<GraphingWindow>("graph", frameStore);
///   registry.show<FrameInfoWindow>("frameinfo", model->getListReference());
class WindowRegistry : public QObject
{
    Q_OBJECT
public:
    explicit WindowRegistry(QObject *parent = nullptr);
    ~WindowRegistry();

    /// Show (or create & show) a window identified by `key`.
    /// The factory `fn` is called only on first access.
    template <typename Win, typename Factory>
    Win *show(const QString &key, Factory fn)
    {
        if (auto *w = mSingletons.value(key)) {
            w->show();
            w->raise();
            w->activateWindow();
            return qobject_cast<Win *>(w);
        }
        Win *win = fn();
        if (win) {
            win->setAttribute(Qt::WA_DeleteOnClose, false);
            mSingletons.insert(key, win);
            // Remove from registry when the window is closed by the user
            connect(win, &QWidget::destroyed, this, [this, key] {
                mSingletons.remove(key);
            });
            win->show();
        }
        return win;
    }

    /// Create a new *multi-instance* graph window.  These are tracked
    /// separately so `closeAll()` can clean them up.
    template <typename Win, typename Factory>
    Win *createGraphWindow(Factory fn)
    {
        Win *win = fn();
        if (win) {
            win->setAttribute(Qt::WA_DeleteOnClose, false);
            mGraphWindows.append(win);
            connect(win, &QWidget::destroyed, this, [this, win] {
                mGraphWindows.removeAll(win);
            });
        }
        return win;
    }

    /// Return a previously-created singleton window, or nullptr.
    template <typename Win = QDialog>
    Win *window(const QString &key) const
    {
        return qobject_cast<Win *>(mSingletons.value(key));
    }

    /// Close and delete all windows tracked by this registry.
    void closeAll();

    /// Return the list of multi-instance graph windows.
    QList<QDialog *> graphWindows() const { return mGraphWindows; }

private:
    QHash<QString, QDialog *> mSingletons;
    QList<QDialog *>          mGraphWindows;
};

#endif // WINDOWREGISTRY_H

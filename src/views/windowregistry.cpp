#include "windowregistry.h"
#include <QWidget>

WindowRegistry::WindowRegistry(QObject *parent)
    : QObject(parent)
{
}

WindowRegistry::~WindowRegistry()
{
    closeAll();
}

void WindowRegistry::closeAll()
{
    // Close graph windows first (multiple instances)
    for (auto *w : mGraphWindows) {
        if (w) {
            w->close();
            w->deleteLater();
        }
    }
    mGraphWindows.clear();

    // Then close singletons
    for (auto it = mSingletons.begin(); it != mSingletons.end(); ++it) {
        if (it.value()) {
            it.value()->close();
            it.value()->deleteLater();
        }
    }
    mSingletons.clear();
}

#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

#include <QObject>
#include <QVector>
#include <QDir>
#include <QPluginLoader>
#include "iplugin.h"

/// PluginManager discovers, loads, and manages IProtocolPlugin instances.
///
/// Plugins are .dll (or .so/.dylib) files placed in the `plugins/`
/// directory relative to the application executable.
///
/// Usage:
///   PluginManager mgr;
///   mgr.loadAll();
///   for (auto *p : mgr.plugins()) { ... }
class PluginManager : public QObject
{
    Q_OBJECT

public:
    explicit PluginManager(QObject *parent = nullptr);
    ~PluginManager();

    /// Scan the plugins directory and load all valid plugins.
    int loadAll(const QString &pluginDir = QString());

    /// Unload all plugins.
    void unloadAll();

    /// List of currently loaded plugin instances.
    QVector<IProtocolPlugin *> plugins() const;

    /// Find a plugin by name.
    IProtocolPlugin *findByName(const QString &name) const;

    /// Number of loaded plugins.
    int count() const;

signals:
    void pluginLoaded(const QString &name);
    void pluginUnloaded(const QString &name);
    void pluginError(const QString &message);

private:
    struct LoadedPlugin {
        QPluginLoader   *loader = nullptr;
        IProtocolPlugin *instance = nullptr;
        QString          filePath;
    };

    QVector<LoadedPlugin> mLoaded;
};

#endif // PLUGINMANAGER_H

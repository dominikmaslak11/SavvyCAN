#include "pluginmanager.h"
#include <QCoreApplication>
#include <QDebug>

PluginManager::PluginManager(QObject *parent)
    : QObject(parent)
{
}

PluginManager::~PluginManager()
{
    unloadAll();
}

int PluginManager::loadAll(const QString &pluginDir)
{
    QString dirPath = pluginDir;
    if (dirPath.isEmpty()) {
        dirPath = QCoreApplication::applicationDirPath() + "/plugins";
    }

    QDir dir(dirPath);
    if (!dir.exists()) {
        emit pluginError(QString("Plugin directory not found: %1").arg(dirPath));
        return 0;
    }

    int count = 0;
    const auto entries = dir.entryInfoList(QDir::Files);
    for (const auto &fi : entries) {
        // Only load .dll (Win), .so (Linux), .dylib (macOS)
        QString suffix = fi.suffix().toLower();
        if (suffix != "dll" && suffix != "so" && suffix != "dylib")
            continue;

        auto *loader = new QPluginLoader(fi.absoluteFilePath());
        QObject *instance = loader->instance();

        if (!instance) {
            qWarning() << "PluginManager: failed to load" << fi.fileName()
                       << "-" << loader->errorString();
            emit pluginError(QString("Failed to load %1: %2")
                .arg(fi.fileName(), loader->errorString()));
            delete loader;
            continue;
        }

        auto *plugin = qobject_cast<IProtocolPlugin *>(instance);
        if (!plugin) {
            qWarning() << "PluginManager:" << fi.fileName()
                       << "does not implement IProtocolPlugin";
            loader->unload();
            delete loader;
            continue;
        }

        if (!plugin->initialize()) {
            qWarning() << "PluginManager:" << plugin->name() << "failed to initialize";
            loader->unload();
            delete loader;
            continue;
        }

        LoadedPlugin lp;
        lp.loader = loader;
        lp.instance = plugin;
        lp.filePath = fi.absoluteFilePath();
        mLoaded.append(lp);

        qInfo() << "Plugin loaded:" << plugin->name() << "v" << plugin->version()
                << "by" << plugin->author();
        emit pluginLoaded(plugin->name());
        ++count;
    }

    return count;
}

void PluginManager::unloadAll()
{
    for (auto &lp : mLoaded) {
        if (lp.instance) {
            emit pluginUnloaded(lp.instance->name());
            lp.instance->shutdown();
        }
        if (lp.loader) {
            lp.loader->unload();
            delete lp.loader;
        }
    }
    mLoaded.clear();
}

QVector<IProtocolPlugin *> PluginManager::plugins() const
{
    QVector<IProtocolPlugin *> result;
    for (const auto &lp : mLoaded)
        result.append(lp.instance);
    return result;
}

IProtocolPlugin *PluginManager::findByName(const QString &name) const
{
    for (const auto &lp : mLoaded) {
        if (lp.instance && lp.instance->name() == name)
            return lp.instance;
    }
    return nullptr;
}

int PluginManager::count() const
{
    return mLoaded.size();
}

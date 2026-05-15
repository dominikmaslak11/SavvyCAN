#ifndef IPROTOCOLPLUGIN_H
#define IPROTOCOLPLUGIN_H

#include <QObject>
#include <QString>
#include <QVector>
#include <QJsonObject>
#include "can_structs.h"

/// Interface for dynamically-loadable protocol decoder plugins.
///
/// Plugins implement this interface and are loaded from
/// the `plugins/` directory by PluginManager.
///
/// Each plugin can:
///   - Decode raw CAN frames into structured data
///   - Provide metadata (name, version, author)
///   - Filter which frames it's interested in
class IProtocolPlugin
{
public:
    virtual ~IProtocolPlugin() = default;

    /// Human-readable plugin name.
    virtual QString name() const = 0;

    /// Plugin version string (e.g. "1.0.0").
    virtual QString version() const = 0;

    /// Author or organization name.
    virtual QString author() const = 0;

    /// Short description of what the plugin does.
    virtual QString description() const = 0;

    /// Returns true if this plugin can decode the given frame.
    virtual bool canDecode(const CANFrame &frame) = 0;

    /// Decode a single CAN frame.  Returns a list of decoded entries,
    /// each being a key-value map (signal name → value).
    virtual QVector<QJsonObject> decode(const CANFrame &frame) = 0;

    /// Called when the plugin is loaded.  Use for initialization.
    virtual bool initialize() { return true; }

    /// Called when the plugin is unloaded.  Use for cleanup.
    virtual void shutdown() {}
};

#define IProtocolPlugin_iid "com.savvycan.IProtocolPlugin"

Q_DECLARE_INTERFACE(IProtocolPlugin, IProtocolPlugin_iid)

#endif // IPROTOCOLPLUGIN_H

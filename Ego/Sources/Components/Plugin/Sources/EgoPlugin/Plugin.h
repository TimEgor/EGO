#pragma once

#include "EgoCore/Patterns/NonCopyable.h"
#include "EgoCore/Pointer/Pointer.h"
#include "EgoCore/RTTI/RTTI.h"

#include "ExternalPlugin.h"
#include "PluginModule.h"

namespace ego
{
    using PluginID = uint64_t;
    inline constexpr PluginID InvalidPluginID = 0;

    using PluginType = rtti::TypeMetaInfoID;

    inline PluginType GetPluginType(const char* _pluginTypeName)
    {
        return rtti::GetTypeMetaInfoID(_pluginTypeName);
    }

    static constexpr PluginID CalcPluginID(PluginModuleID _moduleId, PluginType _pluginType)
    {
        return static_cast<PluginID>(_moduleId) << 32 | _pluginType;
    }

    class Plugin : public NonCopyable
    {
    public:
        Plugin(const PluginModulePointer& _module, PluginType _pluginType);

        ~Plugin() override;

        virtual void onLoaded() {}
        virtual void onUnloaded() {}

        PluginModulePointer getModule() const;
        PluginID getPluginID() const;

        virtual PluginType getType() const = 0;
        virtual const char* getTypeName() const = 0;

        EGO_RTTI_VIRTUAL_BASE(Plugin);

    private:
        PluginModulePointer m_module;
        PluginType m_loadedType;
    };

    struct PluginDeleter final
    {
        void operator()(Plugin* _plugin) const;
    };

    EGO_POINTER(Plugin);
    EGO_WEAK_POINTER(Plugin);
} // namespace ego

#define EGO_PLUGIN_TYPE_INFO()                                                                                                                                                     \
    static const char* GetPluginTypeName()                                                                                                                                         \
    {                                                                                                                                                                              \
        return GetMetaInfoTypeName();                                                                                                                                              \
    }                                                                                                                                                                              \
                                                                                                                                                                                   \
    static ego::PluginType GetPluginType()                                                                                                                                         \
    {                                                                                                                                                                              \
        return GetMetaInfoID();                                                                                                                                                    \
    }

#define EGO_PLUGIN(_TYPE, ...)                                                                                                                                                     \
    EGO_RTTI_VIRTUAL(_TYPE, __VA_ARGS__);                                                                                                                                          \
    EGO_PLUGIN_TYPE_INFO();                                                                                                                                                        \
                                                                                                                                                                                   \
    virtual const char* getTypeName() const override                                                                                                                               \
    {                                                                                                                                                                              \
        return GetPluginTypeName();                                                                                                                                                \
    }                                                                                                                                                                              \
                                                                                                                                                                                   \
    virtual ego::PluginType getType() const override                                                                                                                               \
    {                                                                                                                                                                              \
        return GetPluginType();                                                                                                                                                    \
    }

#define EGO_PLUGIN_TYPE(_PLUGIN) (_PLUGIN::GetPluginType())
#define EGO_PLUGIN_TYPE_NAME(_PLUGIN) (_PLUGIN::GetPluginTypeName())

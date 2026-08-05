#pragma once

#include <mutex>
#include <unordered_map>

#include "EgoCore/Assert/Assert.h"
#include "EgoCore/Patterns/NonInstanceable.h"

#include "Plugin.h"
#include "PluginLoader.h"

namespace ego
{
    class PluginLoader;
    class FileName;

    class PluginController final : public EnableSharedFromThis<PluginController>
    {
        using ModuleContainer = std::unordered_map<PluginModuleID, PluginModuleWeakPointer>;
        using PluginContainer = std::unordered_map<PluginID, PluginWeakPointer>;

    public:
        class PluginControllerAccessor final : public NonInstanceable
        {
            friend struct PluginModuleDeleter;
            friend struct PluginDeleter;

            static void ReleasePluginModule(PluginController& _controller, PluginModule* _pluginModule);
            static void ReleasePlugin(PluginController& _controller, Plugin* _plugin);
        };

        PluginController() = default;
        ~PluginController()
        {
            release();
        }

        bool init();

        FileName selectPluginModule(const char* _pluginTypeName);

        PluginPointer loadPlugin(const FileName& _moduleName, const char* _pluginTypeName)
        {
            if (!_pluginTypeName || _pluginTypeName[0] == '\0')
            {
                return nullptr;
            }

            return loadPlugin(_moduleName, GetPluginType(_pluginTypeName), _pluginTypeName);
        }

        template <typename TPlugin>
        FileName selectPluginModule()
        {
            static_assert(std::is_base_of_v<Plugin, TPlugin>);
            return selectPluginModule(TPlugin::GetPluginTypeName());
        }

        template <typename TPlugin = Plugin>
        SharedPointer<TPlugin> loadPlugin(const FileName& _moduleName)
        {
            static_assert(std::is_base_of_v<Plugin, TPlugin>);
            PluginPointer plugin = loadPlugin(_moduleName, TPlugin::GetPluginType(), TPlugin::GetPluginTypeName());
            EGO_CHECK_RETURN_NULL(plugin);

            EGO_ASSERT((rtti::IsObjectBasedOn<TPlugin>(*plugin)));

            return StaticPointerCast<TPlugin>(plugin);
        }

        template <typename TPlugin>
        SharedPointer<TPlugin> loadPlugin()
        {
            static_assert(std::is_base_of_v<Plugin, TPlugin>);

            const FileName moduleName = selectPluginModule<TPlugin>();
            EGO_CHECK_RETURN_NULL(moduleName);

            return loadPlugin<TPlugin>(moduleName);
        }

    protected:
        void unloadModule(PluginModule* _module);
        void unloadPlugin(Plugin* _plugin);

    private:
        friend class PluginSubsystem;

        void release();

        PluginPointer loadPlugin(const FileName& _moduleName, PluginType _pluginType, const char* _pluginTypeName);

        static PluginModuleID GetModuleID(const FileName& _moduleName);

        mutable std::recursive_mutex m_mutex;

        ModuleContainer m_modules;
        PluginContainer m_plugins;

        PluginLoaderPointer m_loader;
    };

    EGO_POINTER(PluginController);
    EGO_WEAK_POINTER(PluginController);

    PluginControllerPointer GetPluginControllerPointer();
} // namespace ego

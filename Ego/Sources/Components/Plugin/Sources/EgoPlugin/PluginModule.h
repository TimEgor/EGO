#pragma once

#include <vector>

#include "EgoCore/FileName/FileName.h"
#include "EgoCore/Patterns/Singleton.h"
#include "EgoCore/Reference/Pointer.h"

namespace ego
{
    class PluginModuleBindingBridge;

    using PluginModuleID = uint32_t;
    inline constexpr PluginModuleID InvalidPluginModuleID = 0;

    struct PluginModuleInfo
    {
        FileName m_modulePath;
        void* m_handle = nullptr;
        PluginModuleID m_moduleId = InvalidPluginModuleID;
    };

    class PluginModule final
    {
    public:
        PluginModule(const PluginModuleInfo& _info);
        virtual ~PluginModule();

        const PluginModuleInfo& getInfo() const { return m_info; }

    private:
        PluginModuleInfo m_info;
    };

    EGO_POINTER(PluginModule);
    EGO_WEAK_POINTER(PluginModule);

    using PluginModuleInitializerFunction = void(*)(const PluginModuleBindingBridge&);
    using PluginModuleReleaserFunction = void(*)();

    class PluginModuleCore final : public Singleton<PluginModuleCore>
    {
    public:
        PluginModuleCore() = default;

        void init(const PluginModuleBindingBridge& _bindings);
        void release();

        void setInfo(const PluginModuleInfo& _info);
        const PluginModuleInfo& getInfo() const;

        void addInitializer(PluginModuleInitializerFunction _function);
        void addReleaser(PluginModuleReleaserFunction _function);

    private:
        std::vector<PluginModuleInitializerFunction> m_initializers;
        std::vector<PluginModuleReleaserFunction> m_releasers;

        PluginModuleInfo m_info;
        bool m_isInfoSet = false;
    };

    class PluginModuleRegistrator final : public NonCopyable
    {
    public:
        PluginModuleRegistrator(PluginModuleInitializerFunction _init, PluginModuleReleaserFunction _release);
    };
}

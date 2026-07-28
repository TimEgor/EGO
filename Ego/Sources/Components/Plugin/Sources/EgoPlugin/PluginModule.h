#pragma once

#include <vector>

#include "EgoCore/FileName/FileName.h"
#include "EgoCore/Patterns/NonCopyable.h"
#include "EgoCore/Pointer/Pointer.h"

namespace ego
{
    class PluginController;
    EGO_POINTER(PluginController);
    EGO_WEAK_POINTER(PluginController);

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
        PluginModule(const PluginModuleInfo& _info, const PluginControllerWeakPointer& _pluginController);
        virtual ~PluginModule();

        const PluginModuleInfo& getInfo() const
        {
            return m_info;
        }

        PluginControllerPointer getPluginController() const;

    private:
        PluginControllerWeakPointer m_pluginController;
        PluginModuleInfo m_info;
    };

    struct PluginModuleDeleter final
    {
        void operator()(PluginModule* _pluginModule) const;
    };

    EGO_POINTER(PluginModule);
    EGO_WEAK_POINTER(PluginModule);

    using PluginModuleInitializerFunction = void (*)();
    using PluginModuleReleaserFunction = void (*)();

    class PluginModuleState final
    {
    public:
        PluginModuleState() = default;

        void init();
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

    PluginModuleState& GetPluginModuleState();
} // namespace ego

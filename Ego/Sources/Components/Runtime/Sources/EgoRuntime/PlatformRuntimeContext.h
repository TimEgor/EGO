#pragma once

#include "EgoCore/Context/GlobalContext.h"

namespace ego
{
    class PluginController;

    EGO_POINTER(PluginController);
} // namespace ego

namespace ego::context
{
    class PlatformRuntimeContext final : public GlobalContext
    {
    public:
        PlatformRuntimeContext() = default;
        ~PlatformRuntimeContext() override = default;

        bool init();
        void release();

        PluginControllerPointer getPluginControllerPointer() const;
        PluginController& getPluginController() const;

        EGO_RTTI_VIRTUAL(PlatformRuntimeContext, GlobalContext);

    private:
        PluginControllerPointer m_pluginController = nullptr;
    };

    EGO_POINTER(PlatformRuntimeContext);

    PlatformRuntimeContextPointer GetPlatformRuntimeContextPointer();
    PlatformRuntimeContext& GetPlatformRuntimeContext();
} // namespace ego::context

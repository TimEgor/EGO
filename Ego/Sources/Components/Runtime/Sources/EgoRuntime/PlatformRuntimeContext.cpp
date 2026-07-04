#include "PlatformRuntimeContext.h"

#include "EgoCore/Assert/AssertCore.h"
#include "EgoCore/Context/ContextStack.h"
#include "EgoCore/UtilsMacros.h"

#include "EgoRuntime/Plugin/PluginController.h"

bool ego::context::PlatformRuntimeContext::init()
{
    m_pluginController = new PluginController();
    EGO_CHECK_INITIALIZATION(m_pluginController && m_pluginController->init());

    return true;
}

void ego::context::PlatformRuntimeContext::release()
{
    EGO_SAFE_RESET_POINTER_WITH_RELEASING(m_pluginController);
}

ego::PluginControllerPointer ego::context::PlatformRuntimeContext::getPluginControllerPointer() const
{
    return m_pluginController;
}

ego::PluginController& ego::context::PlatformRuntimeContext::getPluginController() const
{
    EGO_ASSERT(m_pluginController);
    return *m_pluginController;
}

ego::context::PlatformRuntimeContextPointer ego::context::GetPlatformRuntimeContextPointer()
{
    return FindGlobalContext<PlatformRuntimeContext>();
}

ego::context::PlatformRuntimeContext& ego::context::GetPlatformRuntimeContext()
{
    const PlatformRuntimeContextPointer platformRuntimeContext = GetPlatformRuntimeContextPointer();
    EGO_ASSERT(platformRuntimeContext);

    return *platformRuntimeContext;
}

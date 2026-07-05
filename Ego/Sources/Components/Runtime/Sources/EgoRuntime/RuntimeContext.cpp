#include "RuntimeContext.h"

#include "EgoCore/Assert/AssertCore.h"
#include "EgoCore/Context/ContextStack.h"
#include "EgoCore/UtilsMacros.h"

#include "EgoRuntime/Event/EventController.h"
#include "EgoRuntime/Input/InputController.h"
#include "EgoRuntime/Plugin/PluginCatalog.h"
#include "EgoRuntime/Resource/ResourceController.h"

bool ego::context::RuntimeContext::init(const RuntimeContext::InitData& _initData)
{
    m_pluginCatalog = new PluginCatalog();
    EGO_CHECK_INITIALIZATION(m_pluginCatalog);

    m_eventController = new EventController();
    EGO_CHECK_INITIALIZATION(m_eventController && m_eventController->init());

    m_inputController = new InputController();
    EGO_CHECK_INITIALIZATION(m_inputController && m_inputController->init());

    EGO_CHECK_INITIALIZATION(initResourceController(_initData));

    return true;
}

void ego::context::RuntimeContext::release()
{
    EGO_SAFE_RESET_POINTER_WITH_RELEASING(m_resourceController);
    EGO_SAFE_RESET_POINTER_WITH_RELEASING(m_inputController);
    EGO_SAFE_RESET_POINTER_WITH_RELEASING(m_eventController);

    if (m_pluginCatalog)
    {
        m_pluginCatalog->clear();
        m_pluginCatalog.reset();
    }
}

bool ego::context::RuntimeContext::initResourceController(const RuntimeContext::InitData& _initData)
{
    EGO_CHECK_RETURN_FALSE(_initData.m_resourceFileSystem);

    m_resourceController = new ResourceController();
    EGO_CHECK_RETURN_FALSE(m_resourceController);
    EGO_CHECK_RETURN_FALSE(m_resourceController->init(_initData.m_resourceJobThreadCount, _initData.m_resourceJobThreadName));
    m_resourceController->addFileSystem(_initData.m_resourceFileSystem);

    return true;
}

ego::PluginCatalogPointer ego::context::RuntimeContext::getPluginCatalogPointer() const
{
    return m_pluginCatalog;
}

ego::PluginCatalog& ego::context::RuntimeContext::getPluginCatalog() const
{
    EGO_ASSERT(m_pluginCatalog);
    return *m_pluginCatalog;
}

ego::EventControllerPointer ego::context::RuntimeContext::getEventControllerPointer() const
{
    return m_eventController;
}

ego::EventController& ego::context::RuntimeContext::getEventController() const
{
    EGO_ASSERT(m_eventController);
    return *m_eventController;
}

ego::InputControllerPointer ego::context::RuntimeContext::getInputControllerPointer() const
{
    return m_inputController;
}

ego::InputController& ego::context::RuntimeContext::getInputController() const
{
    EGO_ASSERT(m_inputController);
    return *m_inputController;
}

ego::ResourceControllerPointer ego::context::RuntimeContext::getResourceControllerPointer() const
{
    return m_resourceController;
}

ego::ResourceController& ego::context::RuntimeContext::getResourceController() const
{
    EGO_ASSERT(m_resourceController);
    return *m_resourceController;
}

ego::context::RuntimeContextPointer ego::context::GetRuntimeContextPointer()
{
    return FindCurrentContext<RuntimeContext>();
}

ego::context::RuntimeContext& ego::context::GetRuntimeContext()
{
    const RuntimeContextPointer runtimeContext = GetRuntimeContextPointer();
    EGO_ASSERT(runtimeContext);

    return *runtimeContext;
}

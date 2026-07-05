#include "Application.h"

#include "EgoCore/Assert/AssertCore.h"
#include "EgoCore/Context/ContextStack.h"
#include "EgoCore/Platform/FileSystem/FileSystem.h"
#include "EgoCore/Profile/Profile.h"
#include "EgoCore/UtilsMacros.h"

#include "EgoRuntime/Plugin/PluginCatalogBuilder.h"
#include "EgoRuntime/RuntimeContext.h"

#include "EgoGraphicHardware/GraphicHardwareContext.h"

ego::application::Application::~Application()
{
    release();
}

bool ego::application::Application::init(const InitData& _initData)
{
    ContextInitData contextInitData;
    contextInitData.m_nativeInstanceHandle = _initData.m_nativeInstanceHandle;

    EGO_CHECK_INITIALIZATION(initContext(contextInitData));
    EGO_CHECK_INITIALIZATION(initRuntime(_initData));

    return true;
}

bool ego::application::Application::initContext(const ContextInitData& _initData)
{
    EGO_CHECK_RETURN_CALL_FALSE(initContextStack(), releaseContextScope());
    EGO_CHECK_RETURN_CALL_FALSE(initPlatformContext(_initData), releaseContextScope());
    EGO_CHECK_RETURN_CALL_FALSE(initDiagnosticContext(), releaseContextScope());
    EGO_CHECK_RETURN_CALL_FALSE(initRuntimeContext(), releaseContextScope());
    EGO_CHECK_RETURN_CALL_FALSE(initPlatformRuntimeContext(), releaseContextScope());

    context::ContextStackCore::GetInstance().setDefaultScope(m_contextScope);
    return true;
}

bool ego::application::Application::initRuntime(const InitData& _initData)
{
    EGO_CHECK_RETURN_CALL_FALSE(initProfilerPlugin(_initData), releaseRuntimeObjects());
    EGO_CHECK_RETURN_CALL_FALSE(initPluginCatalog(_initData), releaseRuntimeObjects());
    EGO_CHECK_RETURN_CALL_FALSE(initApplicationScopedContext(), releaseRuntimeObjects());
    EGO_CHECK_RETURN_CALL_FALSE(initApplicationWindowManager(), releaseRuntimeObjects());
    EGO_CHECK_RETURN_CALL_FALSE(initGraphicHardware(_initData), releaseRuntimeObjects());

    return true;
}

void ego::application::Application::release()
{
    releaseRuntimeObjects();
    releaseContextScope();
}

void ego::application::Application::releaseRuntimeObjects()
{
    releaseGraphicHardwareContext();
    releaseApplicationWindowManager();
    releaseApplicationScopedContext();
    releaseProfilerPlugin();
}

ego::PlatformPointer ego::application::Application::getPlatformPointer() const
{
    return context::GetPlatformPointer();
}

const ego::Platform& ego::application::Application::getPlatform() const
{
    return context::GetPlatform();
}

ego::Platform& ego::application::Application::getPlatform()
{
    return context::GetPlatform();
}

const ego::application::ApplicationWindowManager& ego::application::Application::getApplicationWindowManager() const
{
    EGO_ASSERT(m_applicationWindowManager);
    return *m_applicationWindowManager;
}

ego::application::ApplicationWindowManager& ego::application::Application::getApplicationWindowManager()
{
    EGO_ASSERT(m_applicationWindowManager);
    return *m_applicationWindowManager;
}

bool ego::application::Application::initContextStack()
{
    EGO_CHECK_RETURN_FALSE(!m_contextStack);
    EGO_CHECK_RETURN_FALSE(!m_contextScope);

    m_contextStack = new context::ContextStack();
    EGO_CHECK_RETURN_FALSE(m_contextStack);
    EGO_CHECK_RETURN_FALSE(context::ContextStackCore::GetInstance().init(m_contextStack));
    m_isContextStackInitialized = true;

    m_contextScope = new context::ContextScope();
    EGO_CHECK_RETURN_FALSE(m_contextScope);

    context::ContextStackCore::GetInstance().pushScope(m_contextScope);
    m_isContextScopePushed = true;

    return true;
}

bool ego::application::Application::initPlatformContext(const ContextInitData& _initData)
{
    EGO_CHECK_RETURN_FALSE(!m_platformContext);

    m_platformContext = new context::PlatformContext();
    EGO_CHECK_RETURN_FALSE(m_platformContext);
    EGO_CHECK_RETURN_FALSE(context::ContextStackCore::GetInstance().setGlobalContext(m_platformContext));

    context::PlatformContext::InitData platformContextInitData;
    platformContextInitData.m_nativeInstanceHandle = _initData.m_nativeInstanceHandle;
    EGO_CHECK_RETURN_FALSE(m_platformContext->init(platformContextInitData));

    return true;
}

bool ego::application::Application::initDiagnosticContext()
{
    EGO_CHECK_RETURN_FALSE(!m_diagnosticContext);

    m_diagnosticContext = new context::DiagnosticContext();
    EGO_CHECK_RETURN_FALSE(m_diagnosticContext);
    EGO_CHECK_RETURN_FALSE(context::ContextStackCore::GetInstance().setGlobalContext(m_diagnosticContext));
    m_diagnosticContext->setProfilerController(profile::GetProfilerController());

    return true;
}

bool ego::application::Application::initRuntimeContext()
{
    EGO_CHECK_RETURN_FALSE(!m_runtimeContext);

    m_runtimeContext = new context::RuntimeContext();
    EGO_CHECK_RETURN_FALSE(m_runtimeContext);
    EGO_ASSERT(m_contextScope);
    EGO_CHECK_RETURN_FALSE(m_contextScope);
    m_contextScope->addContext(m_runtimeContext);

    context::RuntimeContext::InitData runtimeContextInitData;
    runtimeContextInitData.m_resourceFileSystem = context::GetPlatform().getFileSystem();
    EGO_CHECK_RETURN_FALSE(m_runtimeContext->init(runtimeContextInitData));

    return true;
}

bool ego::application::Application::initPlatformRuntimeContext()
{
    EGO_CHECK_RETURN_FALSE(!m_platformRuntimeContext);

    m_platformRuntimeContext = new context::PlatformRuntimeContext();
    EGO_CHECK_RETURN_FALSE(m_platformRuntimeContext);
    EGO_CHECK_RETURN_FALSE(context::ContextStackCore::GetInstance().setGlobalContext(m_platformRuntimeContext));
    EGO_CHECK_RETURN_FALSE(m_platformRuntimeContext->init());

    return true;
}

bool ego::application::Application::initApplicationScopedContext()
{
    EGO_CHECK_RETURN_FALSE(!m_applicationContext);

    m_applicationContext = new ApplicationContext();
    EGO_CHECK_RETURN_FALSE(m_applicationContext);

    EGO_ASSERT(m_contextScope);
    EGO_CHECK_RETURN_FALSE(m_contextScope);
    m_contextScope->addContext(m_applicationContext);

    ApplicationContext::InitData contextInitData{*this};
    EGO_CHECK_RETURN_FALSE(m_applicationContext->init(contextInitData));

    return true;
}

bool ego::application::Application::initProfilerPlugin(const InitData& _initData)
{
    m_profilerPlugin = _initData.m_profilerPlugin;
    return true;
}

bool ego::application::Application::initPluginCatalog(const InitData& _initData)
{
    if (!_initData.m_pluginDirectory)
    {
        return true;
    }

    const FileSystemPointer fileSystem = context::GetPlatform().getFileSystem();
    EGO_CHECK_RETURN_FALSE(fileSystem);

    PluginCatalog& pluginCatalog = context::GetRuntimeContext().getPluginCatalog();
    PluginCatalogBuilder::AddPluginsFromPath(pluginCatalog, *fileSystem, _initData.m_pluginDirectory);

    return true;
}

bool ego::application::Application::initApplicationWindowManager()
{
    EGO_CHECK_RETURN_FALSE(!m_applicationWindowManager);

    m_applicationWindowManager = new ApplicationWindowManager();
    EGO_CHECK_RETURN_FALSE(m_applicationWindowManager && m_applicationWindowManager->init());

    return true;
}

bool ego::application::Application::initGraphicHardware(const InitData& _initData)
{
    EGO_CHECK_RETURN_FALSE(!m_graphicHardwareContext);

    m_graphicHardwareContext = new gpu::GraphicHardwareContext();
    EGO_CHECK_RETURN_FALSE(m_graphicHardwareContext);
    EGO_CHECK_RETURN_FALSE(context::ContextStackCore::GetInstance().setGlobalContext(m_graphicHardwareContext));

    gpu::GraphicHardwareContext::InitData graphicHardwareContextInitData;
    graphicHardwareContextInitData.m_graphicHardwarePlugin = _initData.m_graphicHardwarePlugin;
    EGO_CHECK_RETURN_FALSE(m_graphicHardwareContext->init(graphicHardwareContextInitData));

    return true;
}

void ego::application::Application::releaseGraphicHardwareContext()
{
    if (!m_graphicHardwareContext)
    {
        return;
    }

    m_graphicHardwareContext->release();

    if (context::ContextStackCore::GetInstance().getStackPointer() && gpu::GetGraphicHardwareContextPointer().get() == m_graphicHardwareContext.get())
    {
        context::ContextStackCore::GetInstance().removeGlobalContext(m_graphicHardwareContext);
    }

    m_graphicHardwareContext = nullptr;
}

void ego::application::Application::releaseApplicationWindowManager()
{
    EGO_SAFE_RESET_POINTER_WITH_RELEASING(m_applicationWindowManager);
}

void ego::application::Application::releaseApplicationScopedContext()
{
    if (!m_applicationContext)
    {
        return;
    }

    m_applicationContext->release();

    if (m_contextScope)
    {
        m_contextScope->removeContext(m_applicationContext);
    }

    m_applicationContext = nullptr;
}

void ego::application::Application::releaseProfilerPlugin()
{
    m_profilerPlugin = nullptr;
}

void ego::application::Application::releaseContextScope()
{
    const bool hasContextStack = context::ContextStackCore::GetInstance().getStackPointer().get() != nullptr;

    releasePlatformRuntimeContext(hasContextStack);
    releaseRuntimeContext();
    releaseDiagnosticContext(hasContextStack);
    releasePlatformContext(hasContextStack);
    releaseContextStack(hasContextStack);
}

void ego::application::Application::releasePlatformRuntimeContext(bool _hasContextStack)
{
    if (!m_platformRuntimeContext)
    {
        return;
    }

    m_platformRuntimeContext->release();

    if (_hasContextStack && context::GetPlatformRuntimeContextPointer().get() == m_platformRuntimeContext.get())
    {
        context::ContextStackCore::GetInstance().removeGlobalContext(m_platformRuntimeContext);
    }

    m_platformRuntimeContext = nullptr;
}

void ego::application::Application::releaseRuntimeContext()
{
    if (!m_runtimeContext)
    {
        return;
    }

    m_runtimeContext->release();

    if (m_contextScope)
    {
        m_contextScope->removeContext(m_runtimeContext);
    }

    m_runtimeContext = nullptr;
}

void ego::application::Application::releaseDiagnosticContext(bool _hasContextStack)
{
    if (!m_diagnosticContext)
    {
        return;
    }

    m_diagnosticContext->release();

    if (_hasContextStack && context::GetDiagnosticContextPointer().get() == m_diagnosticContext.get())
    {
        context::ContextStackCore::GetInstance().removeGlobalContext(m_diagnosticContext);
    }

    m_diagnosticContext = nullptr;
}

void ego::application::Application::releasePlatformContext(bool _hasContextStack)
{
    if (!m_platformContext)
    {
        return;
    }

    m_platformContext->release();

    if (_hasContextStack && context::GetPlatformContextPointer().get() == m_platformContext.get())
    {
        context::ContextStackCore::GetInstance().removeGlobalContext(m_platformContext);
    }

    m_platformContext = nullptr;
}

void ego::application::Application::releaseContextStack(bool _hasContextStack)
{
    if (_hasContextStack && m_isContextStackInitialized)
    {
        context::ContextStackCore::GetInstance().setDefaultScope(nullptr);

        if (m_isContextScopePushed)
        {
            context::ContextStackCore::GetInstance().popScope(m_contextScope);
            m_isContextScopePushed = false;
        }

        context::ContextStackCore::GetInstance().release();
        m_isContextStackInitialized = false;
    }
    else
    {
        m_isContextStackInitialized = false;
        m_isContextScopePushed = false;
    }

    m_contextScope = nullptr;
    m_contextStack = nullptr;
}

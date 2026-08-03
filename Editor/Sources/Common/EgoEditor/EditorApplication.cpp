#include "EgoEditor/EditorApplication.h"

#include <string>

#include "EgoCore/Parsers/ArgParser/Parser.h"
#include "EgoCore/Parsers/XmlParser/XmlNode.h"
#include "EgoCore/Subsystem/SubsystemRegistry.h"
#include "EgoCore/UtilsMacros.h"

namespace
{
    constexpr auto EditorConfigFileName = "Editor.xml";
} // namespace

ego::editor::EditorApplication::~EditorApplication()
{
    release();
}

bool ego::editor::EditorApplication::init(void* _nativeInstanceHandle, int _argCount, char** _argValues)
{
    release();

    XmlDocument config;
    EGO_CHECK_INITIALIZATION(loadConfig(config));

    CommandLineOptions options;
    ParseCommandLine(_argCount, _argValues, options);

    EGO_CHECK_INITIALIZATION(initApplicationSubsystem(_nativeInstanceHandle, options, config));
    EGO_CHECK_INITIALIZATION(initEngineSubsystem());
    EGO_CHECK_INITIALIZATION(initEditorSubsystem(config));

    return true;
}

void ego::editor::EditorApplication::release()
{
    releaseEditorSubsystem();
    releaseEngineSubsystem();
    releaseApplicationSubsystem();
}

int ego::editor::EditorApplication::run()
{
    const application::ApplicationPointer application = m_applicationSubsystem ? m_applicationSubsystem->getApplicationPointer() : nullptr;
    const engine::EnginePointer engine = m_engineSubsystem ? m_engineSubsystem->getEnginePointer() : nullptr;

    EGO_CHECK_RETURN_VALUE(application && engine && m_editorSubsystem, InitializationFailedExitCode);

    const int exitCode = runMainLoop() ? SuccessExitCode : RuntimeFailedExitCode;
    release();

    return exitCode;
}

bool ego::editor::EditorApplication::initApplicationSubsystem(void* _nativeInstanceHandle, const CommandLineOptions& _options, const XmlDocument& _config)
{
    EGO_CHECK_RETURN_FALSE(!m_applicationSubsystem);

    const XmlNode rootNode = _config.getRootNode();
    EGO_CHECK_RETURN_FALSE(rootNode && rootNode.getNameView() == "Editor");

    const XmlNode applicationNode = rootNode.getChild("Application");
    EGO_CHECK_RETURN_FALSE(applicationNode);

    m_applicationSubsystem = MakePointer<application::ApplicationSubsystem>();
    EGO_CHECK_RETURN_FALSE(m_applicationSubsystem);

    application::Application::InitData initData;
    initData.m_nativeInstanceHandle = _nativeInstanceHandle;
    initData.m_pluginDirectory = FileName(applicationNode.getChildValueOr<std::string>("PluginDirectory", std::string()));
    initData.m_profilerPluginModuleName = FileName(applicationNode.getChildValueOr<std::string>("ProfilerPlugin", std::string()));
    initData.m_graphicHardwarePluginModuleName = ResolveOption(_options.m_graphicHardwarePluginModuleName, applicationNode, "GraphicHardwarePlugin");
    initData.m_enableGraphicHardware = true;
    EGO_CHECK_RETURN_CALL_FALSE(m_applicationSubsystem->init(initData), releaseApplicationSubsystem());

    return true;
}

void ego::editor::EditorApplication::releaseApplicationSubsystem()
{
    EGO_CHECK_RETURN(m_applicationSubsystem);

    const subsystem::SubsystemRegistryPointer subsystemRegistry = subsystem::SubsystemLocator::GetInstance().getRegistryPointer();
    if (subsystemRegistry && subsystemRegistry->findSubsystem(m_applicationSubsystem->getType()).get() == m_applicationSubsystem.get())
    {
        subsystemRegistry->unregisterSubsystem(m_applicationSubsystem);
    }

    EGO_SAFE_RESET_POINTER_WITH_RELEASING(m_applicationSubsystem);
}

bool ego::editor::EditorApplication::loadConfig(XmlDocument& _config) const
{
    return _config.loadFromFile(FileName(EditorConfigFileName));
}

bool ego::editor::EditorApplication::initEngineSubsystem()
{
    EGO_CHECK_RETURN_FALSE(m_applicationSubsystem && m_applicationSubsystem->getApplicationPointer());
    EGO_CHECK_RETURN_FALSE(!m_engineSubsystem);

    m_engineSubsystem = MakePointer<engine::EngineSubsystem>();
    EGO_CHECK_RETURN_FALSE(m_engineSubsystem);
    EGO_CHECK_RETURN_CALL_FALSE(m_engineSubsystem->init(), releaseEngineSubsystem());

    const subsystem::SubsystemRegistryPointer subsystemRegistry = subsystem::SubsystemLocator::GetInstance().getRegistryPointer();
    EGO_CHECK_RETURN_CALL_FALSE(subsystemRegistry && subsystemRegistry->registerSubsystem(m_engineSubsystem), releaseEngineSubsystem());

    return true;
}

void ego::editor::EditorApplication::releaseEngineSubsystem()
{
    EGO_CHECK_RETURN(m_engineSubsystem);

    const subsystem::SubsystemRegistryPointer subsystemRegistry = subsystem::SubsystemLocator::GetInstance().getRegistryPointer();
    if (subsystemRegistry && subsystemRegistry->findSubsystem(m_engineSubsystem->getType()).get() == m_engineSubsystem.get())
    {
        subsystemRegistry->unregisterSubsystem(m_engineSubsystem);
    }

    EGO_SAFE_RESET_POINTER_WITH_RELEASING(m_engineSubsystem);
}

bool ego::editor::EditorApplication::initEditorSubsystem(const XmlDocument& _config)
{
    EGO_CHECK_RETURN_FALSE(m_applicationSubsystem && m_applicationSubsystem->getApplicationPointer());
    EGO_CHECK_RETURN_FALSE(m_engineSubsystem && m_engineSubsystem->getEnginePointer());
    EGO_CHECK_RETURN_FALSE(!m_editorSubsystem);

    m_editorSubsystem = MakePointer<EditorSubsystem>();
    EGO_CHECK_RETURN_FALSE(m_editorSubsystem);

    const subsystem::SubsystemRegistryPointer subsystemRegistry = subsystem::SubsystemLocator::GetInstance().getRegistryPointer();
    EGO_CHECK_RETURN_CALL_FALSE(subsystemRegistry && subsystemRegistry->registerSubsystem(m_editorSubsystem), m_editorSubsystem = nullptr);

    EGO_CHECK_RETURN_CALL_FALSE(m_editorSubsystem->getEditorController().init(_config), releaseEditorSubsystem());

    return true;
}

void ego::editor::EditorApplication::releaseEditorSubsystem()
{
    EGO_CHECK_RETURN(m_editorSubsystem);

    m_editorSubsystem->getEditorController().release();

    const subsystem::SubsystemRegistryPointer subsystemRegistry = subsystem::SubsystemLocator::GetInstance().getRegistryPointer();
    if (subsystemRegistry && subsystemRegistry->findSubsystem(m_editorSubsystem->getType()).get() == m_editorSubsystem.get())
    {
        subsystemRegistry->unregisterSubsystem(m_editorSubsystem);
    }

    m_editorSubsystem = nullptr;
}

bool ego::editor::EditorApplication::runMainLoop()
{
    const application::ApplicationPointer application = m_applicationSubsystem ? m_applicationSubsystem->getApplicationPointer() : nullptr;
    const engine::EnginePointer engine = m_engineSubsystem ? m_engineSubsystem->getEnginePointer() : nullptr;
    EGO_CHECK_RETURN_FALSE(application && engine && m_editorSubsystem);

    EditorController& editorController = m_editorSubsystem->getEditorController();

    while (!application->isExitRequested())
    {
        application->processWindowEvents();
        if (application->isExitRequested() || !editorController.isMainSurfaceValid())
        {
            break;
        }

        application->updateInputDevices();

        EGO_CHECK_RETURN_FALSE(engine->tick());
    }

    return true;
}

ego::FileName ego::editor::EditorApplication::ResolveOption(const FileName& _option, const XmlNode& _configNode, const char* _configName)
{
    if (_option)
    {
        return _option;
    }

    return FileName(_configNode.getChildValueOr<std::string>(_configName, std::string()));
}

void ego::editor::EditorApplication::ParseCommandLine(int _argCount, char** _argValues, CommandLineOptions& _options)
{
    std::string graphicHardwarePluginModuleName;

    ArgParser argParser;
    argParser.addOptionValue("--graphicHardware", graphicHardwarePluginModuleName);

    argParser.parse(_argCount, _argValues);

    _options.m_graphicHardwarePluginModuleName = graphicHardwarePluginModuleName;
}

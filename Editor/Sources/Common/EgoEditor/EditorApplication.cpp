#include "EgoEditor/EditorApplication.h"

#include <string>

#include "EgoCore/Assert/Assert.h"
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
    EGO_CHECK_INITIALIZATION_ASSERT_MESSAGE(application::Application::init(), "Failed to initialize the application.");

    XmlDocument config;
    EGO_CHECK_INITIALIZATION_ASSERT_MESSAGE(loadConfig(config), "Failed to load the editor configuration.");

    CommandLineOptions options;
    ParseCommandLine(_argCount, _argValues, options);

    EGO_CHECK_INITIALIZATION_ASSERT_MESSAGE(initRuntimeSubsystem(_nativeInstanceHandle, options, config), "Failed to initialize the editor runtime subsystem.");
    EGO_CHECK_INITIALIZATION_ASSERT_MESSAGE(initEngineSubsystem(), "Failed to initialize the editor engine subsystem.");
    EGO_CHECK_INITIALIZATION_ASSERT_MESSAGE(initEditorSubsystem(config), "Failed to initialize the editor subsystem.");

    return true;
}

void ego::editor::EditorApplication::release()
{
    releaseEditorSubsystem();
    releaseEngineSubsystem();
    releaseRuntimeSubsystem();
    application::Application::release();
}

int ego::editor::EditorApplication::run()
{
    const bool isReadyToRun =
        m_runtimeSubsystem && m_runtimeSubsystem->getRuntimePointer() && m_engineSubsystem && m_engineSubsystem->getEnginePointer() && m_editorSubsystem;
    EGO_CHECK_RETURN_VALUE(isReadyToRun, InitializationFailedExitCode);

    const int exitCode = runMainLoop() ? SuccessExitCode : RuntimeFailedExitCode;
    release();

    return exitCode;
}

bool ego::editor::EditorApplication::initRuntimeSubsystem(void* _nativeInstanceHandle, const CommandLineOptions& _options, const XmlDocument& _config)
{
    EGO_CHECK_RETURN_FALSE(!m_runtimeSubsystem);

    const XmlNode rootNode = _config.getRootNode();
    EGO_CHECK_RETURN_FALSE(rootNode && rootNode.getNameView() == "Editor");

    const XmlNode runtimeNode = rootNode.getChild("Runtime");
    EGO_CHECK_RETURN_FALSE(runtimeNode);

    m_runtimeSubsystem = MakePointer<runtime::RuntimeSubsystem>();
    EGO_CHECK_RETURN_FALSE(m_runtimeSubsystem);

    runtime::Runtime::InitData initData;
    initData.m_nativeInstanceHandle = _nativeInstanceHandle;
    initData.m_pluginDirectory = FileName(runtimeNode.getChildValueOr<std::string>("PluginDirectory", std::string()));
    initData.m_profilerPluginModuleName = FileName(runtimeNode.getChildValueOr<std::string>("ProfilerPlugin", std::string()));
    initData.m_graphicHardwarePluginModuleName = ResolveOption(_options.m_graphicHardwarePluginModuleName, runtimeNode, "GraphicHardwarePlugin");
    initData.m_enableGraphicHardware = true;
    EGO_CHECK_RETURN_CALL_FALSE(m_runtimeSubsystem->init(initData), releaseRuntimeSubsystem());

    const subsystem::SubsystemRegistryPointer subsystemRegistry = subsystem::SubsystemLocator::GetInstance().getRegistryPointer();
    EGO_CHECK_RETURN_CALL_FALSE(subsystemRegistry && subsystemRegistry->registerSubsystem(m_runtimeSubsystem), releaseRuntimeSubsystem());

    return true;
}

void ego::editor::EditorApplication::releaseRuntimeSubsystem()
{
    const subsystem::SubsystemRegistryPointer subsystemRegistry = subsystem::SubsystemLocator::GetInstance().getRegistryPointer();
    if (subsystemRegistry && m_runtimeSubsystem && subsystemRegistry->findSubsystem(m_runtimeSubsystem->getType()).get() == m_runtimeSubsystem.get())
    {
        subsystemRegistry->unregisterSubsystem(m_runtimeSubsystem);
    }

    m_runtimeSubsystem = nullptr;
}

bool ego::editor::EditorApplication::loadConfig(XmlDocument& _config) const
{
    return _config.loadFromFile(FileName(EditorConfigFileName));
}

bool ego::editor::EditorApplication::initEngineSubsystem()
{
    EGO_CHECK_RETURN_FALSE(m_runtimeSubsystem && m_runtimeSubsystem->getRuntimePointer());
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
    const subsystem::SubsystemRegistryPointer subsystemRegistry = subsystem::SubsystemLocator::GetInstance().getRegistryPointer();
    if (subsystemRegistry && m_engineSubsystem && subsystemRegistry->findSubsystem(m_engineSubsystem->getType()).get() == m_engineSubsystem.get())
    {
        subsystemRegistry->unregisterSubsystem(m_engineSubsystem);
    }

    m_engineSubsystem = nullptr;
}

bool ego::editor::EditorApplication::initEditorSubsystem(const XmlDocument& _config)
{
    EGO_CHECK_RETURN_FALSE(m_runtimeSubsystem && m_runtimeSubsystem->getRuntimePointer());
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
    const subsystem::SubsystemRegistryPointer subsystemRegistry = subsystem::SubsystemLocator::GetInstance().getRegistryPointer();
    if (subsystemRegistry && m_editorSubsystem && subsystemRegistry->findSubsystem(m_editorSubsystem->getType()).get() == m_editorSubsystem.get())
    {
        subsystemRegistry->unregisterSubsystem(m_editorSubsystem);
    }

    m_editorSubsystem = nullptr;
}

bool ego::editor::EditorApplication::runMainLoop()
{
    const runtime::RuntimePointer runtime = m_runtimeSubsystem ? m_runtimeSubsystem->getRuntimePointer() : nullptr;
    const engine::EnginePointer engine = m_engineSubsystem ? m_engineSubsystem->getEnginePointer() : nullptr;
    EGO_CHECK_RETURN_FALSE(runtime && engine && m_editorSubsystem);

    EditorController& editorController = m_editorSubsystem->getEditorController();

    while (!runtime->isExitRequested())
    {
        runtime->processWindowEvents();
        if (runtime->isExitRequested() || !editorController.isMainSurfaceValid())
        {
            break;
        }

        runtime->updateInputDevices();

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

#include "EditorApplication.h"

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
    if (!loadConfig(config))
    {
        release();
        return false;
    }

    CommandLineOptions options;
    ParseCommandLine(_argCount, _argValues, options);

    if (!initApplication(_nativeInstanceHandle, options, config) || !initEngine() || !initEditorSubsystem(config))
    {
        release();
        return false;
    }

    return true;
}

void ego::editor::EditorApplication::release()
{
    releaseEditorSubsystem();
    releaseEngine();
    releaseApplication();
}

int ego::editor::EditorApplication::run()
{
    if (!m_application || !m_engine || !m_editorSubsystem || !m_editorSubsystem->isInitialized())
    {
        return InitializationFailedExitCode;
    }

    const int exitCode = runMainLoop() ? SuccessExitCode : RuntimeFailedExitCode;
    release();

    return exitCode;
}

bool ego::editor::EditorApplication::initApplication(void* _nativeInstanceHandle, const CommandLineOptions& _options, const XmlDocument& _config)
{
    EGO_CHECK_RETURN_FALSE(!m_application);

    const XmlNode rootNode = _config.getRootNode();
    EGO_CHECK_RETURN_FALSE(rootNode && rootNode.getNameView() == "Editor");

    const XmlNode applicationNode = rootNode.getChild("Application");
    EGO_CHECK_RETURN_FALSE(applicationNode);

    m_application = new application::Application();
    EGO_CHECK_RETURN_FALSE(m_application);

    application::Application::InitData initData;
    initData.m_nativeInstanceHandle = _nativeInstanceHandle;
    initData.m_pluginDirectory = FileName(applicationNode.getChildValueOr<std::string>("PluginDirectory", std::string()));
    initData.m_profilerPluginModuleName = FileName(applicationNode.getChildValueOr<std::string>("ProfilerPlugin", std::string()));
    initData.m_graphicHardwarePluginModuleName = ResolveOption(_options.m_graphicHardwarePluginModuleName, applicationNode, "GraphicHardwarePlugin");
    initData.m_enableGraphicHardware = true;
    EGO_CHECK_RETURN_FALSE(m_application->init(initData));

    return true;
}

void ego::editor::EditorApplication::releaseApplication()
{
    EGO_SAFE_RESET_POINTER_WITH_RELEASING(m_application);
}

bool ego::editor::EditorApplication::loadConfig(XmlDocument& _config) const
{
    return _config.loadFromFile(FileName(EditorConfigFileName));
}

bool ego::editor::EditorApplication::initEngine()
{
    EGO_CHECK_RETURN_FALSE(m_application && !m_engine);

    m_engine = new engine::Engine();
    EGO_CHECK_RETURN_FALSE(m_engine);

    return m_engine->init();
}

void ego::editor::EditorApplication::releaseEngine()
{
    EGO_SAFE_RESET_POINTER_WITH_RELEASING(m_engine);
}

bool ego::editor::EditorApplication::initEditorSubsystem(const XmlDocument& _config)
{
    EGO_CHECK_RETURN_FALSE(m_application && m_engine && !m_editorSubsystem);

    m_editorSubsystem = MakePointer<EditorSubsystem>();
    EGO_CHECK_RETURN_FALSE(m_editorSubsystem);

    const subsystem::SubsystemRegistryPointer subsystemRegistry = subsystem::SubsystemLocator::GetInstance().getRegistryPointer();
    if (!subsystemRegistry || !subsystemRegistry->registerSubsystem(m_editorSubsystem))
    {
        m_editorSubsystem = nullptr;
        return false;
    }

    if (!m_editorSubsystem->init(m_application, m_engine, _config))
    {
        releaseEditorSubsystem();
        return false;
    }

    return true;
}

void ego::editor::EditorApplication::releaseEditorSubsystem()
{
    if (!m_editorSubsystem)
    {
        return;
    }

    m_editorSubsystem->release();

    const subsystem::SubsystemRegistryPointer subsystemRegistry = subsystem::SubsystemLocator::GetInstance().getRegistryPointer();
    if (subsystemRegistry && subsystemRegistry->findSubsystem(m_editorSubsystem->getType()).get() == m_editorSubsystem.get())
    {
        subsystemRegistry->unregisterSubsystem(m_editorSubsystem);
    }

    m_editorSubsystem = nullptr;
}

bool ego::editor::EditorApplication::runMainLoop()
{
    EGO_CHECK_RETURN_FALSE(m_application && m_engine && m_editorSubsystem && m_editorSubsystem->isInitialized());

    while (!m_application->isExitRequested())
    {
        m_application->processWindowEvents();
        if (m_application->isExitRequested() || !m_editorSubsystem->isSurfaceValid())
        {
            break;
        }

        m_application->updateInputDevices();
        m_editorSubsystem->update();

        EGO_CHECK_RETURN_FALSE(m_engine->tick());
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

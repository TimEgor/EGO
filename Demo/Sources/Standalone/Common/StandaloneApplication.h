#pragma once

#include <string>

#include "EgoCore/Patterns/NonCopyable.h"

#include "EgoEngine/Engine.h"
#include "EgoEngine/EngineSession.h"
#include "EgoEngine/Project/Project.h"

#include "EgoApplication/Application.h"
#include "EgoApplication/Engine/Gui/Viewport/ApplicationGuiViewportSystem.h"
#include "EgoApplication/Window/ApplicationWindow.h"

namespace ego::demo::standalone
{
    class StandaloneApplication final : public NonCopyable
    {
    public:
        static constexpr int SuccessExitCode = 0;
        static constexpr int InitializationFailedExitCode = 1;
        static constexpr int RuntimeFailedExitCode = 2;

        StandaloneApplication() = default;
        ~StandaloneApplication() override;

        bool init(void* _nativeInstanceHandle, int _argCount, char** _argValues);
        void release();

        int run();

    private:
        struct CommandLineOptions final
        {
            std::string m_pluginDirectoryPath;
            std::string m_profilerPluginName;
            std::string m_renderPluginModuleName;
            std::string m_guiRenderPluginModuleName;
            std::string m_graphicHardwarePluginModuleName;
            std::string m_projectFilePath;
        };

        bool initApplication(void* _nativeInstanceHandle, const CommandLineOptions& _options);
        void releaseApplication();

        bool initEngine(const CommandLineOptions& _options);
        void releaseEngine();

        bool runMainLoop();

        void parseCommandLine(int _argCount, char** _argValues, CommandLineOptions& _options) const;
        bool fillEngineSessionInitData(const CommandLineOptions& _options, engine::EngineSession::InitData& _sessionInitData);
        bool loadProject(const FileName& _projectFileName, engine::ProjectPointer& _project) const;
        static WindowDesc CreateMainWindowDesc();
        FileName selectProjectFile() const;

        application::ApplicationPointer m_application = nullptr;

        engine::EnginePointer m_engine = nullptr;
        engine::EngineSessionPointer m_engineSession = nullptr;

        application::ApplicationWindowWeakPointer m_mainWindow;
        application::ApplicationGuiViewportSystemPointer m_guiViewportSystem = nullptr;
    };
} // namespace ego::demo::standalone

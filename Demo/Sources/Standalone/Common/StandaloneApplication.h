#pragma once

#include <string>

#include "EgoCore/Patterns/NonCopyable.h"

#include "EgoEvent/EventController.h"

#include "EgoApplication/Application.h"
#include "EgoApplication/Window/ApplicationWindow.h"

#include "EgoEngine/Application/Input/EngineWindowInputBinding.h"
#include "EgoEngine/Application/Presentation/EngineWindowPresentation.h"
#include "EgoEngine/Engine.h"
#include "EgoEngine/EngineSession.h"
#include "EgoEngine/Project/Project.h"

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
        bool registerMainWindowEvents(const application::ApplicationWindowPointer& _mainWindow);
        void unregisterMainWindowEvents();

        static WindowDesc CreateMainWindowDesc();
        FileName selectProjectFile() const;

        application::ApplicationPointer m_application = nullptr;

        engine::EnginePointer m_engine = nullptr;
        engine::EngineSessionPointer m_engineSession = nullptr;

        application::ApplicationWindowWeakPointer m_mainWindow;
        engine::EngineWindowPresentationPointer m_mainWindowPresentation = nullptr;
        engine::EngineWindowInputBindingPointer m_mainWindowInputBinding = nullptr;
        EventCallbackID m_mainWindowDestroyingEventCallbackID = InvalidEventCallbackID;
    };
} // namespace ego::demo::standalone

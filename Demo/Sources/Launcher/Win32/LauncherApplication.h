#pragma once

#include <string>

#include "EgoCore/FileName/FileName.h"

#include "EgoApplication/Application.h"
#include "EgoApplication/Window/Window.h"

#include "EgoEngine/Graphic/Presenter/WindowGraphicPresenter.h"
#include "EgoFramework/Framework.h"

namespace ego::engine
{
    class Engine;
} // namespace ego::engine

namespace ego
{
    struct WindowDestroyingEvent;
    struct WindowSystemQuitRequestedEvent;
} // namespace ego

namespace ego::demo::launcher
{
    class LauncherApplication final :
        public application::Application
    {
    public:
        LauncherApplication() = default;
        ~LauncherApplication();

        static constexpr int SuccessExitCode = 0;
        static constexpr int FrameworkCoreInitializationFailedExitCode = 1;
        static constexpr int ProjectLoadingFailedExitCode = 2;
        static constexpr int ApplicationInitializationFailedExitCode = 3;
        static constexpr int ApplicationCoreInitializationFailedExitCode = ApplicationInitializationFailedExitCode;
        static constexpr int FrameworkInitializationFailedExitCode = 10;
        static constexpr int WindowInitializationFailedExitCode = 11;

        int run(void* _nativeInstanceHandle, int _argCount, char** _argValues);

    private:
        struct CommandLineOptions final
        {
            std::string m_platformPluginModuleName;
            std::string m_windowSystemPluginModuleName;
            std::string m_profilerPluginModuleName;
            std::string m_renderPluginModuleName;
            std::string m_renderHardwarePluginModuleName;
            std::string m_projectFilePath;
        };

        int runInternal(void* _nativeInstanceHandle, int _argCount, char** _argValues);
        bool parseCommandLine(int _argCount, char** _argValues, CommandLineOptions& _options) const;
        void fillFrameworkInitData(void* _nativeInstanceHandle, const CommandLineOptions& _options, framework::Framework::InitData& _frameworkInitData) const;
        void fillApplicationInitData(void* _nativeInstanceHandle, const CommandLineOptions& _options, application::Application::InitData& _applicationInitData) const;

        bool loadProject(const CommandLineOptions& _options, framework::ProjectPointer& _project) const;
        FileName selectProjectFile() const;
        bool loadProjectFile(const FileName& _fileName, framework::ProjectPointer& _project) const;

        int initFramework(const framework::Framework::InitData& _frameworkInitData);
        int initApplication(const application::Application::InitData& _applicationInitData);
        bool initWindowRuntime();
        bool initWindowEventCallbacks();
        bool initGraphicPresenter();
        void runWindowLoop();
        bool prepareMainWindow();
        bool isMainWindowValid() const;
        void releaseWindowEventCallbacks();
        void releaseRuntime();
        void releaseApplication();
        void releaseFramework();
        void onMainWindowDestroying(const WindowDestroyingEvent& _event);
        void onWindowSystemQuitRequested(const WindowSystemQuitRequestedEvent&);

        framework::FrameworkPointer m_framework = nullptr;
        WindowPointer m_mainWindow = nullptr;
        WindowGraphicPresenterPointer m_graphicPresenter = nullptr;
        EventCallbackID m_windowDestroyingEventCallbackID = InvalidEventCallbackID;
        EventCallbackID m_windowSystemQuitRequestedEventCallbackID = InvalidEventCallbackID;
        bool m_frameworkCoreInitialized = false;
    };

    EGO_POINTER(LauncherApplication);
} // namespace ego::demo::launcher

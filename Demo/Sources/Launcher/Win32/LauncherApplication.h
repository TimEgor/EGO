#pragma once

#include <string>

#include "EgoCore/FileName/FileName.h"

#include "EgoEngine/Graphic/Presenter/WindowGraphicPresenter.h"
#include "EgoFramework/Framework.h"

namespace ego::engine
{
    class Engine;
} // namespace ego::engine

namespace ego::demo::launcher
{
    class LauncherApplication final
    {
    public:
        LauncherApplication() = default;
        ~LauncherApplication();

        int run(void* _nativeInstanceHandle, int _argCount, char** _argValues);

    private:
        struct CommandLineOptions final
        {
            std::string m_platformPluginModuleName;
            std::string m_profilerPluginModuleName;
            std::string m_renderPluginModuleName;
            std::string m_renderHardwarePluginModuleName;
            std::string m_projectFilePath;
        };

        static constexpr int SuccessExitCode = 0;
        static constexpr int FrameworkCoreInitializationFailedExitCode = 1;
        static constexpr int ProjectLoadingFailedExitCode = 2;
        static constexpr int FrameworkInitializationFailedExitCode = 10;
        static constexpr int WindowInitializationFailedExitCode = 11;

        bool parseCommandLine(int _argCount, char** _argValues, CommandLineOptions& _options) const;
        void fillFrameworkInitData(void* _nativeInstanceHandle, const CommandLineOptions& _options, framework::Framework::InitData& _frameworkInitData) const;

        bool loadProject(const CommandLineOptions& _options, framework::ProjectPointer& _project) const;
        FileName selectProjectFile() const;
        bool loadProjectFile(const FileName& _fileName, framework::ProjectPointer& _project) const;

        int initFramework(const framework::Framework::InitData& _frameworkInitData);
        bool initWindowRuntime();
        void runWindowLoop() const;
        bool prepareMainWindow(engine::Engine& _engine) const;
        bool isMainWindowValid(const engine::Engine& _engine) const;
        void releaseFramework();

        framework::FrameworkPointer m_framework = nullptr;
        WindowGraphicPresenterPointer m_graphicPresenter = nullptr;
        bool m_frameworkCoreInitialized = false;
    };
} // namespace ego::demo::launcher

#pragma once

#include <string>

#include "EgoApplication/Application.h"

#include "EgoRuntime/RuntimeSubsystem.h"
#include "EgoRuntime/Presentation/PresenterProvider.h"

#include "EgoEngine/EngineSession.h"
#include "EgoEngine/EngineSubsystem.h"
#include "EgoEngine/Project/Project.h"

namespace ego::demo::launcher
{
    class LauncherApplication final : public application::Application
    {
    public:
        static constexpr int SuccessExitCode = 0;
        static constexpr int InitializationFailedExitCode = 1;
        static constexpr int RuntimeFailedExitCode = 2;

        LauncherApplication() = default;
        ~LauncherApplication() override;

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

        bool initRuntimeSubsystem(void* _nativeInstanceHandle, const CommandLineOptions& _options);
        void releaseRuntimeSubsystem();

        bool initEngineSubsystem(const CommandLineOptions& _options);
        void releaseEngineSubsystem();

        bool runMainLoop();

        void parseCommandLine(int _argCount, char** _argValues, CommandLineOptions& _options) const;
        bool fillEngineSessionInitData(const CommandLineOptions& _options, engine::EngineSession::InitData& _sessionInitData);
        bool loadProject(const FileName& _projectFileName, engine::ProjectPointer& _project) const;
        static runtime::PresentationDesc CreateMainPresentationDesc();
        FileName selectProjectFile() const;

        runtime::RuntimeSubsystemPointer m_runtimeSubsystem = nullptr;
        engine::EngineSubsystemPointer m_engineSubsystem = nullptr;
        engine::EngineSessionPointer m_engineSession = nullptr;

        PlatformSurfacePointer m_mainSurface = nullptr;
    };
} // namespace ego::demo::launcher

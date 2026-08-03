#pragma once

#include <string>

#include "EgoCore/Patterns/NonCopyable.h"

#include "EgoApplication/ApplicationSubsystem.h"
#include "EgoApplication/Presentation/PresenterProvider.h"

#include "EgoEngine/EngineSession.h"
#include "EgoEngine/EngineSubsystem.h"
#include "EgoEngine/Project/Project.h"

namespace ego::demo::launcher
{
    class LauncherApplication final : public NonCopyable
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

        bool initApplicationSubsystem(void* _nativeInstanceHandle, const CommandLineOptions& _options);
        void releaseApplicationSubsystem();

        bool initEngineSubsystem(const CommandLineOptions& _options);
        void releaseEngineSubsystem();

        bool runMainLoop();

        void parseCommandLine(int _argCount, char** _argValues, CommandLineOptions& _options) const;
        bool fillEngineSessionInitData(const CommandLineOptions& _options, engine::EngineSession::InitData& _sessionInitData);
        bool loadProject(const FileName& _projectFileName, engine::ProjectPointer& _project) const;
        static application::PresentationDesc CreateMainPresentationDesc();
        FileName selectProjectFile() const;

        application::ApplicationSubsystemPointer m_applicationSubsystem = nullptr;
        engine::EngineSubsystemPointer m_engineSubsystem = nullptr;
        engine::EngineSessionPointer m_engineSession = nullptr;

        PlatformSurfacePointer m_mainSurface = nullptr;
    };
} // namespace ego::demo::launcher

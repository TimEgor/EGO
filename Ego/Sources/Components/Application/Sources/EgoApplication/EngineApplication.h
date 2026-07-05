#pragma once

#include <string>

#include "EgoCore/FileName/FileName.h"

#include "EgoEngine/Graphic/Presenter/WindowGraphicPresenter.h"

#include "EgoEngineFramework/EngineFramework.h"

#include "Application.h"
#include "Window/ApplicationWindow.h"

namespace ego::application
{
    struct ApplicationQuitRequestedEvent;
    struct ApplicationWindowDestroyingEvent;
} // namespace ego::application

namespace ego::application
{
    class EngineApplication : public Application
    {
    public:
        EngineApplication() = default;
        ~EngineApplication() override;

    protected:
        struct CommandLineOptions final
        {
            std::string m_pluginDirectoryPath;
            std::string m_profilerPluginModuleName;
            std::string m_renderPluginModuleName;
            std::string m_graphicHardwarePluginModuleName;
            std::string m_projectFilePath;
        };

        bool parseCommandLine(int _argCount, char** _argValues, CommandLineOptions& _options) const;
        bool fillApplicationInitData(void* _nativeInstanceHandle, const CommandLineOptions& _options, InitData& _applicationInitData) const;
        bool fillEngineFrameworkInitData(const CommandLineOptions& _options, engine_framework::EngineFramework::InitData& _engineFrameworkInitData) const;

        bool loadProject(const CommandLineOptions& _options, engine_framework::ProjectPointer& _project) const;
        bool loadProjectFile(const FileName& _fileName, engine_framework::ProjectPointer& _project) const;

        bool initApplicationContext(const ContextInitData& _applicationContextInitData);
        bool initApplicationRuntime(const InitData& _applicationInitData);
        bool initEngineFramework(const engine_framework::EngineFramework::InitData& _engineFrameworkInitData);
        bool initWindowRuntime();
        bool createGraphicPresenter();
        bool initGraphicPresenter();

        void runEngineWindowLoop();

        bool isMainWindowValid() const;
        void releaseApplicationWindowEventCallbacks();
        void releaseGraphicPresenter();
        void releaseApplicationRuntime();
        void releaseEngineFramework();

        context::ContextScopePointer getEngineFrameworkContextScope() const;

        virtual WindowDesc createMainWindowDesc() const = 0;
        virtual FileName selectProjectFile() const;
        virtual GraphicPresenterPointer getEngineGraphicPresenter() const;
        virtual bool useOwnEngineFrameworkContextScope() const;

        engine_framework::EngineFrameworkPointer m_engineFramework = nullptr;
        ApplicationWindowPointer m_mainWindow = nullptr;
        WindowGraphicPresenterPointer m_graphicPresenter = nullptr;

    private:
        bool fillGuiViewportDesc(engine::Engine::InitData& _engineInitData) const;
        bool initEngineFrameworkContextScope();
        bool prepareMainWindow();
        bool initApplicationWindowEventCallbacks();
        void onMainWindowDestroying(const ApplicationWindowDestroyingEvent& _event);
        void onApplicationQuitRequested(const ApplicationQuitRequestedEvent&);

        context::ContextScopePointer m_engineFrameworkContextScope = nullptr;
        EventCallbackID m_applicationWindowDestroyingEventCallbackID = InvalidEventCallbackID;
        EventCallbackID m_applicationQuitRequestedEventCallbackID = InvalidEventCallbackID;
    };

    EGO_POINTER(EngineApplication);
} // namespace ego::application

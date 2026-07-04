#pragma once

#include "EgoCore/Context/ContextScope.h"
#include "EgoCore/Context/ContextStack.h"
#include "EgoCore/Context/DiagnosticContext.h"
#include "EgoCore/Context/PlatformContext.h"
#include "EgoCore/FileName/FileName.h"
#include "EgoCore/Platform/Platform.h"
#include "EgoCore/Reference/Pointer.h"

#include "EgoRuntime/PlatformRuntimeContext.h"
#include "EgoRuntime/RuntimeContext.h"

#include "ApplicationContext.h"
#include "Window/WindowSystem.h"

namespace ego
{
    class ProfilerPlugin;
    EGO_POINTER(ProfilerPlugin);
} // namespace ego

namespace ego::gpu
{
    class GraphicHardwareContext;
    class GraphicHardwarePlugin;

    EGO_POINTER(GraphicHardwareContext);
    EGO_POINTER(GraphicHardwarePlugin);
} // namespace ego::gpu

namespace ego::application
{
    class Application
    {
    public:
        struct ContextInitData final
        {
            void* m_nativeInstanceHandle = nullptr;
        };

        struct InitData final
        {
            void* m_nativeInstanceHandle = nullptr;
            FileName m_pluginDirectory;
            ProfilerPluginPointer m_profilerPlugin = nullptr;
            gpu::GraphicHardwarePluginPointer m_graphicHardwarePlugin = nullptr;
        };

        Application() = default;
        virtual ~Application();

        bool init(const InitData& _initData);
        bool initContext(const ContextInitData& _initData);
        bool initRuntime(const InitData& _initData);
        void release();

        PlatformPointer getPlatformPointer() const;
        const Platform& getPlatform() const;
        Platform& getPlatform();

        const WindowSystem& getWindowSystem() const;
        WindowSystem& getWindowSystem();

    private:
        bool initContextStack();
        bool initPlatformContext(const ContextInitData& _initData);
        bool initDiagnosticContext();
        bool initRuntimeContext();
        bool initPlatformRuntimeContext();

        bool initProfilerPlugin(const InitData& _initData);
        bool initPluginCatalog(const InitData& _initData);
        bool initApplicationScopedContext();
        bool initWindowSystem(const InitData& _initData);
        bool initGraphicHardware(const InitData& _initData);

        void releaseRuntimeObjects();
        void releaseGraphicHardwareContext();
        void releaseWindowSystem();
        void releaseApplicationScopedContext();
        void releaseProfilerPlugin();

        void releaseContextScope();
        void releasePlatformRuntimeContext(bool _hasContextStack);
        void releaseRuntimeContext();
        void releaseDiagnosticContext(bool _hasContextStack);
        void releasePlatformContext(bool _hasContextStack);
        void releaseContextStack(bool _hasContextStack);

        context::PlatformContextPointer m_platformContext = nullptr;
        context::DiagnosticContextPointer m_diagnosticContext = nullptr;
        context::RuntimeContextPointer m_runtimeContext = nullptr;
        context::PlatformRuntimeContextPointer m_platformRuntimeContext = nullptr;
        ApplicationContextPointer m_applicationContext = nullptr;
        context::ContextScopePointer m_contextScope = nullptr;
        context::ContextStackPointer m_contextStack = nullptr;
        bool m_isContextStackInitialized = false;
        bool m_isContextScopePushed = false;
        ProfilerPluginPointer m_profilerPlugin = nullptr;
        WindowSystemPointer m_windowSystem = nullptr;
        gpu::GraphicHardwareContextPointer m_graphicHardwareContext = nullptr;
    };

    EGO_POINTER(Application);
} // namespace ego::application

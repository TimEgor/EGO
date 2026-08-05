#pragma once

#include "EgoCore/Event/EventController.h"
#include "EgoCore/FileName/FileName.h"
#include "EgoCore/Patterns/NonCopyable.h"
#include "EgoCore/Pointer/Pointer.h"
#include "EgoCore/Subsystem/Subsystem.h"

#include "EgoPlugin/Catalog/PluginCatalog.h"

namespace ego
{
    class DiagnosticSubsystem;
    class EventSubsystem;
    class InputController;
    class PlatformSubsystem;
    class PluginSubsystem;
    class ResourceSubsystem;

    EGO_POINTER(DiagnosticSubsystem);
    EGO_POINTER(EventSubsystem);
    EGO_POINTER(InputController);
    EGO_POINTER(PlatformSubsystem);
    EGO_POINTER(PluginSubsystem);
    EGO_POINTER(ResourceSubsystem);
} // namespace ego

namespace ego::gpu
{
    class GraphicHardwareSubsystem;

    EGO_POINTER(GraphicHardwareSubsystem);
} // namespace ego::gpu

namespace ego::runtime
{
    class Runtime;
    class RuntimeInputKeyProvider;
    class RuntimeProfiler;
    class PresenterProvider;
    struct RuntimeQuitRequestedEvent;

    EGO_POINTER(Runtime);
    EGO_POINTER(RuntimeInputKeyProvider);
    EGO_POINTER(RuntimeProfiler);
    EGO_POINTER(PresenterProvider);

    class Runtime final : public NonCopyable
    {
    public:
        struct InitData final
        {
            void* m_nativeInstanceHandle = nullptr;
            FileName m_pluginDirectory;
            FileName m_profilerPluginModuleName;
            FileName m_graphicHardwarePluginModuleName;
            bool m_enableGraphicHardware = false;
            bool m_enableWindowing = true;
        };

        Runtime();
        ~Runtime() override;

        bool init(const InitData& _initData);

        const PresenterProviderPointer& getPresenterProviderPointer() const;
        InputControllerPointer getInputControllerPointer() const;

        void processWindowEvents();
        void updateInputDevices();

        void requestExit();
        bool isExitRequested() const;

    private:
        void release();

        bool initDiagnosticSubsystem();
        bool initEventSubsystem();
        bool initPlatformSubsystem(void* _nativeInstanceHandle);
        bool initPluginSubsystem(const FileName& _pluginDirectory);
        bool initRuntimeProfiler(const FileName& _pluginModuleName);
        bool initGraphicHardwareSubsystem(const FileName& _pluginModuleName);
        bool initResourceSubsystem();

        void releaseResourceSubsystem();
        void releaseGraphicHardwareSubsystem();
        void releaseRuntimeProfiler();
        void releasePluginSubsystem();
        void releasePlatformSubsystem();
        void releaseEventSubsystem();
        void releaseDiagnosticSubsystem();

        bool initInputController();
        void releaseInputController();

        bool registerSubsystem(const subsystem::SubsystemPointer& _subsystem);
        void unregisterSubsystem(const subsystem::SubsystemPointer& _subsystem);

        bool registerPluginDirectory(const FileName& _pluginDirectory);
        void unregisterPluginDirectory();
        bool registerGraphicResourceProvider();

        bool initWindowing();
        void releaseWindowing();
        void handleQuitRequested(const RuntimeQuitRequestedEvent& _event);

        DiagnosticSubsystemPointer m_diagnosticSubsystem = nullptr;
        PlatformSubsystemPointer m_platformSubsystem = nullptr;
        PluginSubsystemPointer m_pluginSubsystem = nullptr;
        PluginCatalog::RegistrationID m_pluginDirectoryRegistrationID = PluginCatalog::InvalidRegistrationID;
        EventSubsystemPointer m_eventSubsystem = nullptr;
        gpu::GraphicHardwareSubsystemPointer m_graphicHardwareSubsystem = nullptr;
        ResourceSubsystemPointer m_resourceSubsystem = nullptr;

        RuntimeProfilerPointer m_runtimeProfiler = nullptr;
        InputControllerPointer m_inputController = nullptr;
        RuntimeInputKeyProviderPointer m_platformInputKeyProvider = nullptr;

        PresenterProviderPointer m_presenterProvider = nullptr;
        EventCallbackID m_runtimeQuitRequestedEventCallbackID = InvalidEventCallbackID;

        bool m_isExitRequested = false;
        bool m_isInitialized = false;
    };
} // namespace ego::runtime

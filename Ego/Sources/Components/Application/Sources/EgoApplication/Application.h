#pragma once

#include "EgoCore/Event/EventController.h"
#include "EgoCore/FileName/FileName.h"
#include "EgoCore/Patterns/NonCopyable.h"
#include "EgoCore/Pointer/Pointer.h"
#include "EgoCore/Subsystem/SubsystemRegistry.h"

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

namespace ego::application
{
    class Application;
    class ApplicationInputKeyProvider;
    class ApplicationSubsystem;
    class ApplicationProfiler;
    class PresenterProvider;
    struct ApplicationQuitRequestedEvent;

    EGO_POINTER(Application);
    EGO_POINTER(ApplicationInputKeyProvider);
    EGO_POINTER(ApplicationSubsystem);
    EGO_POINTER(ApplicationProfiler);
    EGO_POINTER(PresenterProvider);

    class Application final : public NonCopyable
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

        Application();
        ~Application() override;

        bool init(const InitData& _initData, const ApplicationSubsystemPointer& _applicationSubsystem);
        void release();

        const PresenterProviderPointer& getPresenterProviderPointer() const;
        InputControllerPointer getInputControllerPointer() const;

        void processWindowEvents();
        void updateInputDevices();

        void requestExit();
        bool isExitRequested() const;

    private:
        bool initDiagnosticSubsystem();
        bool initEventSubsystem();
        bool initPlatformSubsystem(void* _nativeInstanceHandle);
        bool initPluginSubsystem(const FileName& _pluginDirectory);
        bool initApplicationProfiler(const FileName& _pluginModuleName);
        bool initGraphicHardwareSubsystem(const FileName& _pluginModuleName);
        bool initResourceSubsystem();

        void releaseResourceSubsystem();
        void releaseGraphicHardwareSubsystem();
        void releaseApplicationProfiler();
        void releasePluginSubsystem();
        void releasePlatformSubsystem();
        void releaseEventSubsystem();
        void releaseDiagnosticSubsystem();

        bool initInputController();
        void releaseInputController();

        bool initSubsystemRegistry();
        void releaseSubsystemRegistry();

        bool registerSubsystem(const subsystem::SubsystemPointer& _subsystem);
        void releaseSubsystem(const subsystem::SubsystemPointer& _subsystem);

        bool registerPluginDirectory(const FileName& _pluginDirectory);
        void unregisterPluginDirectory();
        bool registerGraphicResourceProvider();

        bool initWindowing();
        void releaseWindowing();
        void handleQuitRequested(const ApplicationQuitRequestedEvent& _event);

        subsystem::SubsystemRegistryPointer m_subsystemRegistry = nullptr;
        DiagnosticSubsystemPointer m_diagnosticSubsystem = nullptr;
        PlatformSubsystemPointer m_platformSubsystem = nullptr;
        PluginSubsystemPointer m_pluginSubsystem = nullptr;
        PluginCatalog::RegistrationID m_pluginDirectoryRegistrationID = PluginCatalog::InvalidRegistrationID;
        EventSubsystemPointer m_eventSubsystem = nullptr;
        gpu::GraphicHardwareSubsystemPointer m_graphicHardwareSubsystem = nullptr;
        ResourceSubsystemPointer m_resourceSubsystem = nullptr;

        ApplicationProfilerPointer m_applicationProfiler = nullptr;
        InputControllerPointer m_inputController = nullptr;
        ApplicationInputKeyProviderPointer m_platformInputKeyProvider = nullptr;

        PresenterProviderPointer m_presenterProvider = nullptr;
        EventCallbackID m_applicationQuitRequestedEventCallbackID = InvalidEventCallbackID;

        bool m_isExitRequested = false;
    };
} // namespace ego::application

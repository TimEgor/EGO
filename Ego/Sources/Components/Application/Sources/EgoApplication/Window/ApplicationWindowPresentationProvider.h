#pragma once

#include <vector>

#include "EgoCore/Patterns/NonCopyable.h"
#include "EgoCore/Platform/Window/WindowSystem.h"

#include "EgoEvent/EventController.h"

#include "EgoGraphicHardware/GraphicObjects/SwapChain.h"

#include "EgoApplication/Presentation/PresenterProvider.h"

#include "ApplicationWindow.h"
#include "WindowGraphicPresenter.h"

namespace ego::application
{
    struct PresentationSurfaceSizeChangedEvent;

    class ApplicationWindowPresentationProvider final : public PresenterProvider,
                                                        public WindowSystemEventListener,
                                                        public EnableSharedFromThis<ApplicationWindowPresentationProvider>,
                                                        public NonCopyable
    {
    public:
        struct InitData final
        {
            gpu::SwapChainDesc m_swapChainDesc;
        };

        ApplicationWindowPresentationProvider() = default;
        ~ApplicationWindowPresentationProvider() override;

        bool init(const InitData& _initData);
        void release() override;

        Presentation createPresentation(const PresentationDesc& _desc) override;
        bool destroyPresentation(const PresentationSurfacePointer& _surface) override;
        GraphicPresenterPointer findGraphicPresenter(const PresentationSurfacePointer& _surface) const override;
        void processEvents() override;

        EGO_RTTI_VIRTUAL(ApplicationWindowPresentationProvider, PresenterProvider);

    private:
        struct PresentationEntry final
        {
            PresentationSurfacePointer m_surface = nullptr;
            WindowGraphicPresenterPointer m_graphicPresenter = nullptr;
            InstancedEventCallbackID m_sizeChangedCallbackID = InvalidInstancedEventCallbackID;
        };

        using PresentationCollection = std::vector<PresentationEntry>;
        using ApplicationWindowCollection = std::vector<ApplicationWindowPointer>;

        ApplicationWindowPointer createApplicationWindow(const WindowDesc& _desc);

        WindowSystem& getNativeWindowSystem() const;
        bool registerApplicationEvents();
        void unregisterApplicationEvents();
        bool registerWindowSystemEventListener();
        void unregisterWindowSystemEventListener();

        void releasePresentations();
        void releasePresentation(PresentationEntry& _presentation);
        bool removePresentation(const PresentationSurfacePointer& _surface);
        WindowGraphicPresenterPointer createSurfaceGraphicPresenter(
            const PresentationSurfacePointer& _surface,
            InstancedEventCallbackID& _sizeChangedCallbackID);
        WindowGraphicPresenterPointer findSurfaceGraphicPresenter(const PresentationSurfacePointer& _surface) const;

        void releaseApplicationWindows();
        ApplicationWindowPointer findApplicationWindow(const WindowPointer& _nativeWindow) const;
        void removeApplicationWindow(const ApplicationWindowPointer& _window);

        static EventControllerPointer GetEventControllerPointer();

        void onWindowSystemQuitRequested() override;
        void onWindowDestroying(const WindowPointer& _nativeWindow) override;
        void onWindowActivation(const WindowPointer& _nativeWindow, bool _isActive) override;
        void onWindowSizeChanged(const WindowPointer& _nativeWindow, const WindowSize& _prevSize) override;
        void onWindowKeyboardInput(const WindowPointer& _nativeWindow, const WindowKeyboardInputData& _inputData) override;
        void onWindowTextInput(const WindowPointer& _nativeWindow, const WindowTextInputData& _inputData) override;

        void emitApplicationQuitRequested() const;
        void emitPresentationSurfaceDestroying(const ApplicationWindowPointer& _window) const;
        void emitPresentationSurfaceActivation(const ApplicationWindowPointer& _window, bool _isActive) const;
        void emitPresentationSurfaceSizeChanged(const ApplicationWindowPointer& _window, const WindowSize& _previousSize) const;
        void emitPresentationSurfaceKeyboardInput(const ApplicationWindowPointer& _window, const WindowKeyboardInputData& _inputData) const;
        void emitPresentationSurfaceTextInput(const ApplicationWindowPointer& _window, const WindowTextInputData& _inputData) const;

        void handlePresentationSurfaceSizeChanged(const PresentationSurfaceSizeChangedEvent& _event);

        gpu::SwapChainDesc m_swapChainDesc;
        PresentationCollection m_presentations;
        ApplicationWindowCollection m_windows;
        bool m_areApplicationEventsRegistered = false;
        bool m_isWindowSystemEventListenerRegistered = false;
        bool m_isInitialized = false;
    };

    EGO_POINTER(ApplicationWindowPresentationProvider);
} // namespace ego::application

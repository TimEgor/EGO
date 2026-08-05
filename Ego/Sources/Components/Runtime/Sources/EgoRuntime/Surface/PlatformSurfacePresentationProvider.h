#pragma once

#include <vector>

#include "EgoCore/Event/EventController.h"
#include "EgoCore/Patterns/NonCopyable.h"
#include "EgoCore/Platform/Surface/PlatformSurfaceController.h"

#include "EgoGraphicHardware/GraphicObjects/SwapChain.h"

#include "EgoRuntime/Presentation/PresenterProvider.h"
#include "EgoRuntime/Presentation/SurfaceGraphicPresenter.h"

namespace ego::runtime
{
    class PlatformSurfacePresentationProvider final : public PresenterProvider, public NonCopyable
    {
    public:
        struct InitData final
        {
            gpu::SwapChainDesc m_swapChainDesc;
        };

        PlatformSurfacePresentationProvider() = default;
        ~PlatformSurfacePresentationProvider() override;

        bool init(const InitData& _initData);

        Presentation createPresentation(const PresentationDesc& _desc) override;
        bool destroyPresentation(const PlatformSurfacePointer& _surface) override;
        GraphicPresenterPointer findGraphicPresenter(const PlatformSurfacePointer& _surface) const override;
        void processEvents() override;

        EGO_RTTI_VIRTUAL(PlatformSurfacePresentationProvider, PresenterProvider);

    private:
        void release();

        struct SurfaceEventCallbackIDs final
        {
            InstancedEventCallbackID m_closeRequested = InvalidInstancedEventCallbackID;
            InstancedEventCallbackID m_sizeChanged = InvalidInstancedEventCallbackID;
        };

        struct PresentationEntry final
        {
            PlatformSurfacePointer m_surface = nullptr;
            SurfaceGraphicPresenterPointer m_graphicPresenter = nullptr;
            SurfaceEventCallbackIDs m_eventCallbackIDs;
        };

        using PresentationCollection = std::vector<PresentationEntry>;

        PlatformSurfaceController& getSurfaceController() const;
        bool registerSurfaceEvents(PresentationEntry& _presentation);
        void unregisterSurfaceEvents(PresentationEntry& _presentation);

        void releasePresentations();
        void releasePresentation(PresentationEntry& _presentation, bool _destroySurface);
        PresentationCollection::iterator findPresentation(PlatformSurface& _surface);
        PresentationCollection::const_iterator findPresentation(const PlatformSurface& _surface) const;
        SurfaceGraphicPresenterPointer createGraphicPresenter(const PlatformSurface& _surface) const;

        void handleSurfaceCloseRequested(const PlatformSurfaceCloseRequestedEvent& _event);
        void handleSurfaceSizeChanged(const PlatformSurfaceSizeChangedEvent& _event);

        static EventControllerPointer GetEventControllerPointer();

        gpu::SwapChainDesc m_swapChainDesc;
        PresentationCollection m_presentations;
        PlatformSurfacePointer m_mainSurface = nullptr;
        bool m_isInitialized = false;
    };

    EGO_POINTER(PlatformSurfacePresentationProvider);
} // namespace ego::runtime

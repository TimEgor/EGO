#pragma once

#include "EgoCore/Reference/Pointer.h"

#include "EgoGui/Viewport/ViewportBackend.h"

#include "EgoEngine/Graphic/Presenter/GraphicPresenter.h"

namespace ego::engine
{
    enum class EngineViewportPrepareResult
    {
        Ready,
        Unavailable,
        TargetResizeRequired,
        Failed
    };

    class EngineViewportHost
    {
    public:
        EngineViewportHost() = default;
        virtual ~EngineViewportHost() = default;

        virtual EngineViewportPrepareResult prepareForRender() = 0;
        virtual bool resizeRenderTarget() = 0;
        virtual GraphicPresenterPointer getGraphicPresenterPointer() const = 0;
    };

    EGO_POINTER(EngineViewportHost);

    class EngineGuiViewportBackend : public gui::ViewportBackend
    {
    public:
        EngineGuiViewportBackend() = default;
        ~EngineGuiViewportBackend() override = default;

        virtual EngineViewportHostPointer findViewportHost(gui::ViewportID _viewportID) const = 0;
    };

    EGO_POINTER(EngineGuiViewportBackend);
} // namespace ego::engine

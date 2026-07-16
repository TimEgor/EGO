#pragma once

#include "EgoCore/Reference/Pointer.h"

#include "EgoGui/Viewport/GuiViewportBackend.h"

#include "EgoEngine/Graphic/Presenter/GraphicPresenter.h"

namespace ego::engine
{
    enum class EngineViewportPrepareResult
    {
        Ready,
        Unavailable,
        Failed
    };

    class EngineViewportHost
    {
    public:
        EngineViewportHost() = default;
        virtual ~EngineViewportHost() = default;

        virtual EngineViewportPrepareResult prepareForRender() = 0;
        virtual GraphicPresenterPointer getGraphicPresenterPointer() const = 0;
    };

    EGO_POINTER(EngineViewportHost);

    class EngineGuiViewportBackend : public gui::GuiViewportBackend
    {
    public:
        EngineGuiViewportBackend() = default;
        ~EngineGuiViewportBackend() override = default;

        virtual EngineViewportHostPointer findViewportHost(gui::GuiViewportID _viewportID) const = 0;
    };

    EGO_POINTER(EngineGuiViewportBackend);
} // namespace ego::engine

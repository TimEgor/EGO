#pragma once

#include <vector>

#include "EgoCore/Reference/Pointer.h"

#include "EgoGraphicHardware/Presentation/GraphicPresenter.h"

#include "EgoGui/Input/Input.h"

#include "ViewportTypes.h"

namespace ego::gui
{
    using InputEventCollection = std::vector<InputEventPointer>;

    enum class ViewportUpdateStatus
    {
        Alive,
        CloseRequested,
        Lost
    };

    struct ViewportUpdate final
    {
        ViewportUpdateStatus m_status = ViewportUpdateStatus::Lost;
        Size m_size = SizeZero;
        InputEventCollection m_input;
        GraphicPresenterPointer m_graphicPresenter = nullptr;
    };

    class ViewportBackend
    {
    public:
        virtual ~ViewportBackend() = default;

        virtual bool createViewport(const ViewportCreateRequest& _request) = 0;
        virtual void destroyViewport(ViewportID _viewportID) = 0;
        virtual ViewportUpdate pollViewport(ViewportID _viewportID) = 0;
    };

    EGO_POINTER(ViewportBackend);
} // namespace ego::gui

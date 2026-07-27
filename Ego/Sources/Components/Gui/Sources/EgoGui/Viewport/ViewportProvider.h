#pragma once

#include <vector>

#include "EgoCore/Reference/Pointer.h"

#include "EgoGraphicHardware/Presentation/GraphicPresenter.h"

#include "EgoGui/Input/Input.h"

#include "ViewportTypes.h"

namespace ego::gui
{
    using InputEventCollection = std::vector<InputEventOwner>;

    enum class ViewportUpdateStatus
    {
        Alive,
        CloseRequested,
        Lost
    };

    struct ViewportUpdate final
    {
        ViewportUpdateStatus m_status = ViewportUpdateStatus::Lost;
        Position m_position = PositionZero;
        Size m_size = SizeZero;
        // These flags report platform changes that do not acknowledge the latest provider request.
        bool m_positionChanged = false;
        bool m_sizeChanged = false;
        InputEventCollection m_input;
        GraphicPresenterPointer m_graphicPresenter = nullptr;
    };

    class ViewportProvider
    {
    public:
        virtual ~ViewportProvider() = default;

        virtual bool createViewport(const ViewportCreateRequest& _request) = 0;
        virtual void destroyViewport(ViewportID _viewportID) = 0;
        virtual ViewportUpdate pollViewport(ViewportID _viewportID) = 0;
        virtual bool showViewport(ViewportID _viewportID, bool _activate) = 0;
        virtual bool setViewportPosition(ViewportID _viewportID, Position& _position) = 0;
        virtual bool setViewportSize(ViewportID _viewportID, Size& _size) = 0;
        virtual bool setViewportInputPassthrough(ViewportID _viewportID, bool _isEnabled) = 0;
        virtual ViewportID findViewportAtScreenPosition(const Position& _position) const = 0;
    };

    EGO_POINTER(ViewportProvider);
} // namespace ego::gui

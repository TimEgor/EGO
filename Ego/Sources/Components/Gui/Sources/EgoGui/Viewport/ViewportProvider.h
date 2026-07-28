#pragma once

#include <vector>

#include "EgoCore/Pointer/Pointer.h"

#include "EgoGraphicHardware/Presentation/GraphicPresenter.h"

#include "EgoGui/Input/InputEvents.h"

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

    struct ViewportState final
    {
        ViewportUpdateStatus m_status = ViewportUpdateStatus::Lost;
        FloatVector2 m_position = FloatVector2Zero;
        FloatVector2 m_size = FloatVector2Zero;
        GraphicPresenterPointer m_graphicPresenter = nullptr;
        bool m_isFocused = false;
        bool m_isInputPassthroughEnabled = false;
    };

    struct ViewportUpdate final
    {
        ViewportState m_state;
        // These flags report platform changes that do not acknowledge the latest provider request.
        bool m_positionChanged = false;
        bool m_sizeChanged = false;
        InputEventCollection m_input;
    };

    class ViewportProvider
    {
    public:
        virtual ~ViewportProvider() = default;

        virtual bool createViewport(const ViewportCreateRequest& _request) = 0;
        virtual void destroyViewport(ViewportID _viewportID) = 0;
        virtual ViewportState getViewportState(ViewportID _viewportID) const = 0;
        virtual ViewportUpdate pollViewport(ViewportID _viewportID) = 0;
        virtual bool showViewport(ViewportID _viewportID, bool _activate) = 0;
        virtual bool setViewportPosition(ViewportID _viewportID, FloatVector2& _position) = 0;
        virtual bool setViewportSize(ViewportID _viewportID, FloatVector2& _size) = 0;
        virtual bool setViewportInputPassthrough(ViewportID _viewportID, bool _isEnabled) = 0;
    };

    EGO_POINTER(ViewportProvider);
} // namespace ego::gui

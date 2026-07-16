#pragma once

#include <string>

#include "EgoGui/Widgets/GuiWidget.h"

namespace ego::gui
{
    class GuiWindow;
    EGO_POINTER(GuiWindow);
    EGO_WEAK_POINTER(GuiWindow);

    class GuiWindow final : public GuiWidget
    {
    public:
        GuiWindow() = default;

        static GuiWindowPointer Create();

        void setTitle(std::string _title);
        const std::string& getTitle() const;

        void setPosition(const GuiPosition& _position);
        const GuiPosition& getPosition() const;

        void setSize(const GuiSize& _size);
        const GuiSize& getSize() const;

        void setMinimumSize(const GuiSize& _size);
        const GuiSize& getMinimumSize() const;

        void setContent(const GuiWidgetPointer& _content);
        GuiWidgetPointer getContent() const;

        const GuiRect& getWindowRect() const;

        EGO_RTTI_VIRTUAL(GuiWindow, GuiWidget);

    protected:
        GuiEventResult onEvent(const GuiInputEvent& _event) override;
        GuiSize onMeasure(const GuiLayoutContext& _context, const GuiSize& _availableSize) override;
        void onArrange(const GuiLayoutContext& _context, const GuiRect& _rect) override;
        void onPaint(GuiPaintContext& _context) const override;

    private:
        enum class InteractionState
        {
            None,
            Dragging,
            Resizing
        };

        enum class PointerRegion
        {
            None,
            Content,
            TitleBar,
            ResizeGrip
        };

        GuiEventResult processPointerEvent(const GuiInputEvent& _event);
        GuiEventResult dispatchToContent(const GuiInputEvent& _event);
        PointerRegion hitTest(const GuiPosition& _position) const;
        void updateHoveredRegion(const GuiPosition& _position);

        void beginInteraction(InteractionState _state, const GuiPosition& _position);
        void pauseInteraction();
        void updateInteraction(const GuiPosition& _position);
        void endInteraction();
        void updateWindowRects();

        std::string m_title;
        GuiRect m_bounds;
        GuiSize m_minimumSize = GuiSize(120.0f, 80.0f);
        GuiWidgetPointer m_content = nullptr;
        GuiRect m_titleBarRect;
        GuiRect m_contentRect;
        GuiRect m_resizeGripRect;
        GuiPosition m_interactionStartPosition = GuiPositionZero;
        GuiRect m_interactionStartBounds;
        InteractionState m_interactionState = InteractionState::None;
        PointerRegion m_hoveredRegion = PointerRegion::None;
        bool m_isInteractionPaused = false;
    };
} // namespace ego::gui

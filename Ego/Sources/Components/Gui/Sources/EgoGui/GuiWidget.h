#pragma once

#include "EgoCore/Reference/Pointer.h"
#include "EgoCore/RTTI/RTTI.h"

#include "GuiInput.h"
#include "GuiPaintContext.h"
#include "GuiTypes.h"

namespace ego::gui
{
    class GuiWidget
    {
    public:
        GuiWidget() = default;
        virtual ~GuiWidget() = default;

        GuiSize measure(const GuiLayoutContext& _context, const GuiSize& _availableSize);
        void arrange(const GuiLayoutContext& _context, const GuiRect& _rect);
        void paint(GuiPaintContext& _context) const;
        virtual GuiReply handleEvent(const GuiInputEvent& _event);

        void setVisibility(GuiVisibility _visibility);
        GuiVisibility getVisibility() const;
        bool isVisible() const;
        bool isCollapsed() const;

        const GuiSize& getDesiredSize() const;
        const GuiRect& getRect() const;

        EGO_RTTI_VIRTUAL_BASE(GuiWidget);

    protected:
        virtual GuiSize onMeasure(const GuiLayoutContext& _context, const GuiSize& _availableSize);
        virtual void onArrange(const GuiLayoutContext& _context, const GuiRect& _rect);
        virtual void onPaint(GuiPaintContext& _context) const;

    private:
        GuiVisibility m_visibility = GuiVisibility::Visible;
        GuiSize m_desiredSize = GuiSizeZero;
        GuiRect m_rect;
    };

    EGO_POINTER(GuiWidget);
} // namespace ego::gui

#pragma once

#include "EgoGui/Docking/DockingPreview.h"
#include "EgoGui/Widgets/Widget.h"

namespace ego::gui
{
    class DockingOverlay;

    EGO_POINTER(DockingOverlay);

    class DockingOverlay final : public Widget
    {
    public:
        static DockingOverlayPointer Create();

        void setPreview(const DockingPreview& _preview);
        void clearPreview();

        EGO_RTTI_VIRTUAL(DockingOverlay, Widget);

    protected:
        bool hitTest(const Position& _position) const override;
        void drawBaseLayer(PaintContext& _context) const override;

    private:
        DockingOverlay() = default;

        void drawTarget(PaintContext& _context, const DockingTarget& _target, bool _isHovered) const;

        DockingPreview m_preview;
    };
} // namespace ego::gui

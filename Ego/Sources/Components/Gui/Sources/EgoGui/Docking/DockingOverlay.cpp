#include "DockingOverlay.h"

#include "EgoGui/Rendering/PaintContext.h"
#include "EgoGui/Theme/Theme.h"

ego::gui::DockingOverlayPointer ego::gui::DockingOverlay::Create()
{
    return new DockingOverlay();
}

void ego::gui::DockingOverlay::setPreview(const DockingPreview& _preview)
{
    m_preview = _preview;
    invalidateLayout();
}

void ego::gui::DockingOverlay::clearPreview()
{
    setPreview(DockingPreview());
}

bool ego::gui::DockingOverlay::hitTest(const Position&) const
{
    return false;
}

void ego::gui::DockingOverlay::drawBaseLayer(PaintContext& _context) const
{
    const DockingStyle& style = _context.getTheme().m_docking;
    if (m_preview.m_isDropAllowed)
    {
        _context.drawBox(m_preview.m_previewBounds, style.m_previewFill);
    }

    for (const DockingTarget& target : m_preview.m_targets)
    {
        const bool isHovered = m_preview.m_isDropAllowed && target.m_level == m_preview.m_targetLevel && target.m_placement == m_preview.m_placement;
        drawTarget(_context, target, isHovered);
    }
}

void ego::gui::DockingOverlay::drawTarget(PaintContext& _context, const DockingTarget& _target, bool _isHovered) const
{
    if (!_target.m_isVisible || !_target.m_isAvailable)
    {
        return;
    }

    constexpr float BorderThickness = 2.0f;

    const DockingStyle& style = _context.getTheme().m_docking;
    const NormalizedColorRGBA& color = style.m_target.resolve(_isHovered, false);
    const Rect& bounds = _target.m_bounds;
    _context.drawBox(bounds, color);
    _context.drawBorder(bounds, BorderThickness, style.m_targetBorder);

    const float centerX = bounds.m_position.m_x + bounds.m_size.m_x * 0.5f;
    const float centerY = bounds.m_position.m_y + bounds.m_size.m_y * 0.5f;
    if (_target.m_placement == DockingPlacement::Left || _target.m_placement == DockingPlacement::Right)
    {
        _context.drawBox(Rect(centerX - BorderThickness * 0.5f, bounds.getTop(), BorderThickness, bounds.m_size.m_y), style.m_targetBorder);
    }
    else if (_target.m_placement == DockingPlacement::Top || _target.m_placement == DockingPlacement::Bottom)
    {
        _context.drawBox(Rect(bounds.getLeft(), centerY - BorderThickness * 0.5f, bounds.m_size.m_x, BorderThickness), style.m_targetBorder);
    }
}

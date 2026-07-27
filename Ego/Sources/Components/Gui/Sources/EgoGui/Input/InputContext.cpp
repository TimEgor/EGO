#include "InputContext.h"

#include "EgoGui/Viewport/SurfaceRoot.h"
#include "EgoGui/Viewport/ViewportInputContext.h"

namespace
{
    using RootAccessor = ego::gui::SurfaceRoot::InputAccessor;
} // namespace

ego::gui::InputContext ego::gui::InputContext::InputAccessor::Create(
    WindowDrag& _windowDrag,
    const WindowDragContext& _windowDragContext,
    const Position& _screenPosition)
{
    return InputContext(_windowDrag, _windowDragContext, _screenPosition);
}

bool ego::gui::InputContext::InputAccessor::UpdateCapturedWindowDrag(InputContext& _context, const Position& _position)
{
    return _context.updateCapturedWindowDrag(_position);
}

bool ego::gui::InputContext::InputAccessor::FinishCapturedWindowDrag(InputContext& _context, const Position& _position)
{
    return _context.finishCapturedWindowDrag(_position);
}

void ego::gui::InputContext::InputAccessor::CancelWindowDragAfterCaptureFailure(InputContext& _context)
{
    _context.cancelWindowDragAfterCaptureFailure();
}

ego::gui::InputContext::InputContext(WindowDrag& _windowDrag, const WindowDragContext& _windowDragContext, const Position& _screenPosition)
    : m_windowDrag(_windowDrag),
      m_windowDragContext(_windowDragContext),
      m_screenPosition(_screenPosition)
{
}

const ego::gui::Rect& ego::gui::InputContext::getSurfaceBounds() const
{
    return m_windowDragContext.m_root.getLayoutBounds();
}

void ego::gui::InputContext::beginFloatingWindowDrag(const WindowPointer& _window, const Position& _position)
{
    m_windowDrag.beginFloating(m_windowDragContext, _window, _position, m_screenPosition);
}

void ego::gui::InputContext::beginDockedWindowDrag(const WindowPointer& _window, const Position& _position)
{
    m_windowDrag.beginDocked(m_windowDragContext, _window, _position, m_screenPosition);
}

bool ego::gui::InputContext::updateWindowDrag(const WindowPointer& _window, const Position& _position)
{
    return m_windowDrag.update(m_windowDragContext, _window, _position, m_screenPosition);
}

void ego::gui::InputContext::finishWindowDrag(const WindowPointer& _window, const Position& _position)
{
    m_windowDrag.finish(m_windowDragContext, _window, _position, m_screenPosition);
}

void ego::gui::InputContext::cancelWindowDrag(const WindowPointer& _window)
{
    m_windowDrag.cancel(m_windowDragContext, _window);
}

bool ego::gui::InputContext::allowsInteractionOutsideSurface() const
{
    return m_windowDragContext.m_viewportInput.allowsInteractionOutsideSurface();
}

bool ego::gui::InputContext::hasCapturedWindowDrag() const
{
    return m_windowDrag.hasPointerCapture();
}

bool ego::gui::InputContext::updateCapturedWindowDrag(const Position& _position)
{
    if (!hasCapturedWindowDrag())
    {
        return false;
    }

    return updateWindowDrag(m_windowDrag.getWindow(), _position);
}

bool ego::gui::InputContext::finishCapturedWindowDrag(const Position& _position)
{
    const WindowPointer window = m_windowDrag.getWindow();
    if (!hasCapturedWindowDrag() || !window)
    {
        return false;
    }

    finishWindowDrag(window, _position);

    return true;
}

void ego::gui::InputContext::cancelWindowDragAfterCaptureFailure()
{
    const WindowPointer window = m_windowDrag.getWindow();
    if (!window)
    {
        RootAccessor::ClearInteraction(m_windowDragContext.m_root);

        return;
    }

    cancelWindowDrag(window);
    RootAccessor::ClearInteraction(m_windowDragContext.m_root);
}

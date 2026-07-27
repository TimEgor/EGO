#include "ViewportInputContext.h"

#include "ViewportManager.h"

namespace
{
    using ManagerAccessor = ego::gui::ViewportManager::ViewportInputAccessor;
} // namespace

ego::gui::ViewportInputContext ego::gui::ViewportInputContext::ViewportManagerAccessor::Create(ViewportManager& _manager, ViewportID _inputViewportID)
{
    return ViewportInputContext(_manager, _inputViewportID);
}

ego::gui::ViewportInputContext::ViewportInputContext(ViewportManager& _manager, ViewportID _inputViewportID)
    : m_manager(_manager),
      m_inputViewportID(_inputViewportID)
{
}

bool ego::gui::ViewportInputContext::beginWindowDrag(const WindowPointer& _window, const Position& _screenPosition, const Rect& _localWindowBounds)
{
    return ManagerAccessor::BeginWindowDrag(m_manager, m_inputViewportID, _window, _screenPosition, _localWindowBounds);
}

void ego::gui::ViewportInputContext::updateWindowDrag(const WindowPointer& _window, const Position& _screenPosition)
{
    ManagerAccessor::UpdateWindowDrag(m_manager, m_inputViewportID, _window, _screenPosition);
}

bool ego::gui::ViewportInputContext::finishWindowDrag(const WindowPointer& _window, const Position& _screenPosition)
{
    return ManagerAccessor::FinishWindowDrag(m_manager, m_inputViewportID, _window, _screenPosition);
}

void ego::gui::ViewportInputContext::cancelWindowDrag(const WindowPointer& _window)
{
    ManagerAccessor::CancelWindowDrag(m_manager, m_inputViewportID, _window);
}

bool ego::gui::ViewportInputContext::allowsInteractionOutsideSurface() const
{
    return ManagerAccessor::AllowsInteractionOutsideSurface(m_manager, m_inputViewportID);
}

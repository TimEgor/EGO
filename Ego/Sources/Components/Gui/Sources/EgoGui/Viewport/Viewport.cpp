#include "Viewport.h"

#include <cstddef>
#include <memory>

#include "EgoCore/Assert/Assert.h"

#include "EgoGui/Input/InputState.h"
#include "EgoGui/Layout/Layout.h"
#include "EgoGui/Rendering/PaintContext.h"
#include "EgoGui/Widgets/Window.h"

#include "SurfaceRoot.h"
#include "ViewportInputContext.h"
#include "ViewportProvider.h"

namespace
{
    using LayoutAccessor = ego::gui::LayoutContext::ViewportAccessor;
    using PaintAccessor = ego::gui::PaintContext::ViewportAccessor;
    using RootAccessor = ego::gui::SurfaceRoot::ViewportAccessor;
} // namespace

ego::SharedPointer<ego::gui::Viewport> ego::gui::Viewport::ViewportManagerAccessor::Create(ViewportID _id, const ViewportDesc& _desc)
{
    return Viewport::Create(_id, _desc);
}

ego::gui::ViewportID ego::gui::Viewport::ViewportManagerAccessor::GetID(const Viewport& _viewport)
{
    return _viewport.getID();
}

const ego::gui::Position& ego::gui::Viewport::ViewportManagerAccessor::GetPosition(const Viewport& _viewport)
{
    return _viewport.getPosition();
}

const ego::gui::Size& ego::gui::Viewport::ViewportManagerAccessor::GetSize(const Viewport& _viewport)
{
    return _viewport.getSize();
}

ego::GraphicPresenterPointer ego::gui::Viewport::ViewportManagerAccessor::GetGraphicPresenterPointer(const Viewport& _viewport)
{
    return _viewport.getGraphicPresenterPointer();
}

void ego::gui::Viewport::ViewportManagerAccessor::SetPosition(Viewport& _viewport, const Position& _position)
{
    _viewport.setPosition(_position);
}

void ego::gui::Viewport::ViewportManagerAccessor::SetSize(Viewport& _viewport, const Size& _size)
{
    _viewport.setSize(_size);
}

bool ego::gui::Viewport::ViewportManagerAccessor::ContainsWindow(const Viewport& _viewport, const WindowPointer& _window)
{
    return _viewport.containsWindow(_window);
}

size_t ego::gui::Viewport::ViewportManagerAccessor::GetWindowCount(const Viewport& _viewport)
{
    return _viewport.getWindowCount();
}

ego::gui::WindowPointer ego::gui::Viewport::ViewportManagerAccessor::GetWindow(const Viewport& _viewport, size_t _index)
{
    return _viewport.getWindow(_index);
}

size_t ego::gui::Viewport::ViewportManagerAccessor::GetFloatingWindowCount(const Viewport& _viewport)
{
    return _viewport.getFloatingWindowCount();
}

ego::gui::WindowPointer ego::gui::Viewport::ViewportManagerAccessor::GetFloatingWindow(const Viewport& _viewport, size_t _index)
{
    return _viewport.getFloatingWindow(_index);
}

bool ego::gui::Viewport::ViewportManagerAccessor::IsWindowBound(const Viewport& _viewport, const WindowPointer& _window)
{
    return _viewport.isWindowBound(_window);
}

bool ego::gui::Viewport::ViewportManagerAccessor::IsWindowFloating(const Viewport& _viewport, const WindowPointer& _window)
{
    return _viewport.isWindowFloating(_window);
}

bool ego::gui::Viewport::ViewportManagerAccessor::TransferWindowTo(Viewport& _viewport, Viewport& _target, const WindowPointer& _window)
{
    return _viewport.transferWindowTo(_target, _window);
}

void ego::gui::Viewport::ViewportManagerAccessor::CancelWindowInteraction(Viewport& _viewport, const WindowPointer& _window)
{
    _viewport.cancelWindowInteraction(_window);
}

void ego::gui::Viewport::ViewportManagerAccessor::UpdateExternalDragPreview(Viewport& _viewport, const WindowPointer& _window, const Position& _position)
{
    _viewport.updateExternalDragPreview(_window, _position);
}

bool ego::gui::Viewport::ViewportManagerAccessor::DockExternalWindow(Viewport& _viewport, const WindowPointer& _window, const Position& _position)
{
    return _viewport.dockExternalWindow(_window, _position);
}

bool ego::gui::Viewport::ViewportManagerAccessor::IsWindowDragActive(const Viewport& _viewport, const WindowPointer& _window)
{
    return _viewport.isWindowDragActive(_window);
}

void ego::gui::Viewport::ViewportManagerAccessor::ClearExternalDragPreview(Viewport& _viewport)
{
    _viewport.clearExternalDragPreview();
}

void ego::gui::Viewport::ViewportManagerAccessor::Clear(Viewport& _viewport)
{
    _viewport.clear();
}

void ego::gui::Viewport::ViewportManagerAccessor::ApplyViewportUpdate(Viewport& _viewport, const ViewportUpdate& _update)
{
    _viewport.applyViewportUpdate(_update);
}

bool ego::gui::Viewport::ViewportManagerAccessor::UpdateLayout(Viewport& _viewport, const LayoutContext& _layoutContext)
{
    return _viewport.updateLayout(_layoutContext);
}

void ego::gui::Viewport::ViewportManagerAccessor::ProcessInput(Viewport& _viewport, const InputEvent& _input, ViewportInputContext& _inputContext)
{
    _viewport.processInput(_input, _inputContext);
}

void ego::gui::Viewport::ViewportManagerAccessor::Paint(Viewport& _viewport, PaintContext& _paintContext)
{
    _viewport.paint(_paintContext);
}

void ego::gui::Viewport::ViewportManagerAccessor::InvalidateLayout(Viewport& _viewport)
{
    _viewport.invalidateLayout();
}

ego::SharedPointer<ego::gui::Viewport> ego::gui::Viewport::Create(ViewportID _id, const ViewportDesc& _desc)
{
    const ego::SharedPointer<Viewport> viewport = new Viewport(_id, _desc);

    return viewport && viewport->m_root && viewport->m_inputState ? viewport : nullptr;
}

ego::gui::Viewport::Viewport(ViewportID _id, const ViewportDesc& _desc)
    : m_id(_id),
      m_position(_desc.m_position),
      m_size(_desc.m_size),
      m_root(RootAccessor::Create())
{
    if (m_root)
    {
        m_inputState = std::make_unique<InputState>(*m_root);
    }
}

ego::gui::Viewport::~Viewport()
{
    clearInteraction();
    m_inputState.reset();
    if (m_root)
    {
        RootAccessor::ClearWindows(*m_root);
        m_root = nullptr;
    }
}

ego::gui::ViewportID ego::gui::Viewport::getID() const
{
    return m_id;
}

const ego::gui::Position& ego::gui::Viewport::getPosition() const
{
    return m_position;
}

const ego::gui::Size& ego::gui::Viewport::getSize() const
{
    return m_size;
}

ego::GraphicPresenterPointer ego::gui::Viewport::getGraphicPresenterPointer() const
{
    return m_graphicPresenter;
}

void ego::gui::Viewport::setPosition(const Position& _position)
{
    m_position = _position;
}

void ego::gui::Viewport::setSize(const Size& _size)
{
    if (AreEqual(m_size, _size))
    {
        return;
    }

    m_size = _size;
    invalidateLayout();
}

bool ego::gui::Viewport::transferWindowTo(Viewport& _target, const WindowPointer& _window)
{
    const WindowPointer window = _window;
    if (!m_root || !_target.m_root || this == &_target || !window || !RootAccessor::ContainsWindow(*m_root, window) ||
        RootAccessor::IsWindowBound(*m_root, window))
    {
        return false;
    }

    return RootAccessor::TransferWindowTo(*m_root, *_target.m_root, window);
}

void ego::gui::Viewport::cancelWindowInteraction(const WindowPointer& _window)
{
    if (!m_root)
    {
        return;
    }

    InputState& inputState = getInputState();
    if (!inputState.isWindowDragActive(_window))
    {
        return;
    }

    inputState.requestPointerCaptureCancellation();
    inputState.flushPointerCaptureCancellation();
    inputState.cancelWindowDrag(_window);
}

bool ego::gui::Viewport::containsWindow(const WindowPointer& _window) const
{
    return m_root && RootAccessor::ContainsWindow(*m_root, _window);
}

size_t ego::gui::Viewport::getWindowCount() const
{
    return m_root ? RootAccessor::GetWindowCount(*m_root) : 0;
}

ego::gui::WindowPointer ego::gui::Viewport::getWindow(size_t _index) const
{
    return m_root ? RootAccessor::GetWindow(*m_root, _index) : nullptr;
}

size_t ego::gui::Viewport::getFloatingWindowCount() const
{
    return m_root ? RootAccessor::GetFloatingWindowCount(*m_root) : 0;
}

ego::gui::WindowPointer ego::gui::Viewport::getFloatingWindow(size_t _index) const
{
    return m_root ? RootAccessor::GetFloatingWindow(*m_root, _index) : nullptr;
}

bool ego::gui::Viewport::isWindowBound(const WindowPointer& _window) const
{
    return m_root && RootAccessor::IsWindowBound(*m_root, _window);
}

bool ego::gui::Viewport::isWindowFloating(const WindowPointer& _window) const
{
    return m_root && RootAccessor::IsWindowFloating(*m_root, _window);
}

bool ego::gui::Viewport::addWindow(const WindowPointer& _window, bool _isBound)
{
    if (!m_root || !_window)
    {
        return false;
    }

    return RootAccessor::AttachWindow(*m_root, _window, _isBound);
}

bool ego::gui::Viewport::removeWindow(const WindowPointer& _window)
{
    const WindowPointer window = _window;
    if (!m_root || !window || !RootAccessor::ContainsWindow(*m_root, window))
    {
        return false;
    }

    InputState& inputState = getInputState();
    const bool isWindowDragActive = inputState.isWindowDragActive(window);
    inputState.cancelWindowDrag(window);
    if (isWindowDragActive)
    {
        inputState.requestPointerCaptureCancellation();
        inputState.flushPointerCaptureCancellation();
    }

    const WindowPointer removedWindow = RootAccessor::ExtractWindow(*m_root, window);
    if (removedWindow)
    {
        inputState.detachInputState(window);
    }

    return static_cast<bool>(removedWindow);
}

void ego::gui::Viewport::clear()
{
    if (!m_root)
    {
        return;
    }

    while (RootAccessor::GetWindowCount(*m_root) > 0)
    {
        const WindowPointer window = RootAccessor::GetWindow(*m_root, 0);
        if (!removeWindow(window))
        {
            EGO_ASSERT_FAIL_MESSAGE("Failed to remove a window while clearing a viewport.");
            break;
        }
    }

    getInputState().detachInputState();
    RootAccessor::ClearWindows(*m_root);
}

ego::gui::WidgetPointer ego::gui::Viewport::getFocusedWidget() const
{
    if (!m_root)
    {
        return nullptr;
    }

    return m_inputState ? m_inputState->getFocusedWidget() : nullptr;
}

bool ego::gui::Viewport::setDockingEnabled(bool _isEnabled)
{
    if (!m_root)
    {
        return false;
    }

    const bool dockingStateChanged = RootAccessor::IsDockingEnabled(*m_root) != _isEnabled;
    if (dockingStateChanged)
    {
        InputState& inputState = getInputState();
        const WindowPointer draggedWindow = inputState.getDraggedWindow();
        if (draggedWindow)
        {
            inputState.cancelWindowDrag(draggedWindow);
        }

        inputState.requestPointerCaptureCancellation();
        inputState.flushPointerCaptureCancellation();
    }

    return RootAccessor::SetDockingEnabled(*m_root, _isEnabled);
}

bool ego::gui::Viewport::isDockingEnabled() const
{
    return m_root && RootAccessor::IsDockingEnabled(*m_root);
}

ego::gui::DockingSpaceID ego::gui::Viewport::getDefaultDockingSpaceID() const
{
    return m_root ? RootAccessor::GetDefaultDockingSpaceID(*m_root) : InvalidDockingSpaceID;
}

ego::gui::DockingSpaceID ego::gui::Viewport::getWindowDockingSpaceID(const WindowPointer& _window) const
{
    return m_root ? RootAccessor::GetWindowDockingSpaceID(*m_root, _window) : InvalidDockingSpaceID;
}

bool ego::gui::Viewport::moveWindow(const WindowPointer& _window, const DockingLocation& _location)
{
    const WindowPointer window = _window;
    InputState& inputState = getInputState();
    const bool isWindowDragActive = m_root && inputState.isWindowDragActive(window);
    if (!m_root || !RootAccessor::DockWindow(*m_root, window, _location))
    {
        return false;
    }

    inputState.cancelWindowDrag(window);
    inputState.requestPointerCaptureCancellation(isWindowDragActive ? nullptr : window);
    inputState.flushPointerCaptureCancellation();

    return true;
}

bool ego::gui::Viewport::isWindowDragActive(const WindowPointer& _window) const
{
    return m_inputState && m_inputState->isWindowDragActive(_window);
}

void ego::gui::Viewport::applyViewportUpdate(const ViewportUpdate& _update)
{
    m_graphicPresenter = _update.m_graphicPresenter;
    if (_update.m_positionChanged)
    {
        setPosition(_update.m_position);
    }

    if (_update.m_sizeChanged)
    {
        setSize(_update.m_size);
    }
}

bool ego::gui::Viewport::updateLayout(const LayoutContext& _layoutContext)
{
    return stabilize(_layoutContext, getInputState());
}

void ego::gui::Viewport::processInput(const InputEvent& _input, ViewportInputContext& _inputContext)
{
    getInputState().dispatch(_input, _inputContext);
}

void ego::gui::Viewport::clearInteraction()
{
    if (!m_root)
    {
        return;
    }

    getInputState().clear();
}

void ego::gui::Viewport::invalidateLayout()
{
    if (m_root)
    {
        RootAccessor::InvalidateLayout(*m_root);
    }
}

bool ego::gui::Viewport::stabilize(const LayoutContext& _layoutContext, InputState& _inputState)
{
    if (!m_root)
    {
        return false;
    }

    size_t layoutPassCount = 0;
    while (true)
    {
        while (LayoutAccessor::IsLayoutInvalidated(*m_root))
        {
            if (layoutPassCount >= MaximumLayoutPassCount)
            {
                EGO_ASSERT_FAIL_MESSAGE("GUI layout did not stabilize.");

                return false;
            }

            LayoutAccessor::Layout(_layoutContext, *m_root, m_size);
            ++layoutPassCount;
        }

        _inputState.refresh();
        if (!LayoutAccessor::IsLayoutInvalidated(*m_root))
        {
            return true;
        }

        if (layoutPassCount >= MaximumLayoutPassCount)
        {
            EGO_ASSERT_FAIL_MESSAGE("GUI layout did not stabilize after input state refresh.");

            return false;
        }
    }
}

void ego::gui::Viewport::paint(PaintContext& _paintContext)
{
    if (!m_root)
    {
        return;
    }

    PaintAccessor::Paint(_paintContext, *m_root);
}

ego::gui::InputState& ego::gui::Viewport::getInputState()
{
    EGO_ASSERT(m_inputState);

    return *m_inputState;
}

void ego::gui::Viewport::updateExternalDragPreview(const WindowPointer& _window, const Position& _position)
{
    if (m_root)
    {
        RootAccessor::UpdateExternalDragPreview(*m_root, _window, _position);
    }
}

bool ego::gui::Viewport::dockExternalWindow(const WindowPointer& _window, const Position& _position)
{
    return m_root && RootAccessor::DockExternalWindow(*m_root, _window, _position);
}

void ego::gui::Viewport::clearExternalDragPreview()
{
    if (m_root)
    {
        RootAccessor::ClearExternalDragPreview(*m_root);
    }
}

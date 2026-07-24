#include "DockingArea.h"

#include <algorithm>
#include <cmath>

#include "EgoCore/Assert/Assert.h"
#include "EgoCore/RTTI/RTTI.h"

#include "EgoGui/Docking/DockingNode.h"
#include "EgoGui/Docking/DockingSpace.h"
#include "EgoGui/Docking/DockingSplit.h"
#include "EgoGui/Docking/DockingTab.h"
#include "EgoGui/Widgets/Window.h"

ego::gui::DockingAreaPointer ego::gui::DockingArea::Create()
{
    return new DockingArea();
}

bool ego::gui::DockingArea::initializeRoot()
{
    if (m_root)
    {
        return true;
    }

    const DockingSpacePointer root = createSpace();
    if (!root || !attachChild(root))
    {
        return false;
    }

    m_root = root;
    notifyTreeChanged();

    return true;
}

ego::gui::DockingArea::WindowCollection ego::gui::DockingArea::clearWindows()
{
    WindowCollection windows = getWindows();
    for (const WindowPointer& window : windows)
    {
        const WindowPointer releasedWindow = undockWindow(window);
        EGO_ASSERT(releasedWindow);
    }

    notifyTreeChanged();

    return windows;
}

ego::gui::DockingArea::WindowCollection ego::gui::DockingArea::releaseWindows()
{
    WindowCollection windows = clearWindows();
    if (m_root)
    {
        const bool detached = detachChild(m_root);
        EGO_ASSERT(detached);
        m_root = nullptr;
    }

    notifyTreeChanged();

    return windows;
}

ego::gui::DockingNodePointer ego::gui::DockingArea::getRootNode() const
{
    return m_root;
}

ego::gui::DockingSpacePointer ego::gui::DockingArea::getDefaultSpace() const
{
    return findFirstSpace(m_root);
}

ego::gui::DockingSpacePointer ego::gui::DockingArea::findSpace(DockingSpaceID _spaceID) const
{
    return _spaceID != InvalidDockingSpaceID ? findSpace(m_root, _spaceID) : nullptr;
}

ego::gui::DockingSpacePointer ego::gui::DockingArea::findSpaceAt(const Position& _position) const
{
    if (m_root && m_root->getSpaceCount() == 0 && getLayoutBounds().contains(_position))
    {
        return getDefaultSpace();
    }

    return findSpaceAt(m_root, _position);
}

ego::gui::DockingSpacePointer ego::gui::DockingArea::findWindowSpace(const WindowPointer& _window) const
{
    return findWindowSpace(m_root, _window);
}

ego::gui::Rect ego::gui::DockingArea::getDockingBounds(const DockingSpacePointer& _space) const
{
    if (!_space || findSpace(_space->getID()).get() != _space.get())
    {
        return Rect();
    }

    const DockingSpacePointer defaultSpace = getDefaultSpace();
    if (m_root && m_root->getSpaceCount() == 0 && defaultSpace.get() == _space.get())
    {
        return getLayoutBounds();
    }

    return _space->getLayoutBounds();
}

ego::gui::DockingArea::WindowCollection ego::gui::DockingArea::getWindows() const
{
    WindowCollection windows;
    appendWindows(m_root, windows);

    return windows;
}

bool ego::gui::DockingArea::isSplitAvailableAfterRemoving(
    const DockingSpacePointer& _originSpace,
    const DockingSpacePointer& _targetSpace,
    DockingAxis _axis,
    const Size& _rootSize,
    const Size& _minimumSpaceSize,
    float _separatorThickness) const
{
    if (!m_root || !_originSpace || !_targetSpace || _originSpace.get() == _targetSpace.get() || findSpace(_originSpace->getID()).get() != _originSpace.get() ||
        findSpace(_targetSpace->getID()).get() != _targetSpace.get() || _originSpace->getSpaceCount() == 0 || _targetSpace->getSpaceCount() == 0 ||
        !std::isfinite(_rootSize.m_x) || !std::isfinite(_rootSize.m_y) || !std::isfinite(_minimumSpaceSize.m_x) || !std::isfinite(_minimumSpaceSize.m_y) ||
        !std::isfinite(_separatorThickness))
    {
        return false;
    }

    switch (_axis)
    {
    case DockingAxis::Horizontal:
    case DockingAxis::Vertical:
        break;

    default:
        return false;
    }

    const DockingRequirements requirements =
        calculateRequirementsAfterRemoving(m_root, *_originSpace, *_targetSpace, _axis, _minimumSpaceSize, _separatorThickness);
    const Size rootSize((std::max)(0.0f, _rootSize.m_x), (std::max)(0.0f, _rootSize.m_y));

    return requirements.m_spaceCount > 0 && requirements.m_minimumSize.m_x <= rootSize.m_x && requirements.m_minimumSize.m_y <= rootSize.m_y;
}

bool ego::gui::DockingArea::dockWindow(const WindowPointer& _window, const DockingSpacePointer& _targetSpace, DockingPlacement _placement, float _ratio)
{
    if (!_window || !_targetSpace || findSpace(_targetSpace->getID()).get() != _targetSpace.get() || !std::isfinite(_ratio))
    {
        return false;
    }

    switch (_placement)
    {
    case DockingPlacement::Center:
    case DockingPlacement::Left:
    case DockingPlacement::Right:
    case DockingPlacement::Top:
    case DockingPlacement::Bottom:
        break;

    default:
        return false;
    }

    const DockingSpacePointer originSpace = findWindowSpace(_window);
    if (_window->getParent() && !originSpace)
    {
        return false;
    }

    const bool targetsOrigin = originSpace && originSpace.get() == _targetSpace.get();
    if (targetsOrigin && _placement == DockingPlacement::Center)
    {
        return _targetSpace->selectWindow(_window);
    }

    if (targetsOrigin && !_targetSpace->hasNonCollapsedWindowExcept(*_window))
    {
        return false;
    }

    if (_targetSpace->getSpaceCount() == 0)
    {
        _placement = DockingPlacement::Center;
    }

    if (_placement == DockingPlacement::Center)
    {
        if (!originSpace)
        {
            return _targetSpace->addWindow(_window);
        }

        const DockingTabPointer tab = originSpace->releaseTab(_window);
        if (!tab)
        {
            return false;
        }

        if (!_targetSpace->insertTab(tab))
        {
            const bool restored = originSpace->insertTab(tab);
            EGO_ASSERT(restored);

            return false;
        }

        if (originSpace->isEmpty())
        {
            const bool collapsed = collapseEmptySpace(originSpace);
            EGO_ASSERT(collapsed);
        }

        return true;
    }

    const DockingSpacePointer newSpace = createSpace();
    const DockingSplitPointer newSplit = createSplit(_placement, _ratio);
    if (!newSpace || !newSplit || !splitNode(_targetSpace, newSpace, newSplit, _placement))
    {
        return false;
    }

    bool inserted = false;
    if (originSpace)
    {
        const DockingTabPointer tab = originSpace->releaseTab(_window);
        inserted = tab && newSpace->insertTab(tab);
        if (!inserted && tab)
        {
            const bool restored = originSpace->insertTab(tab);
            EGO_ASSERT(restored);
        }
    }
    else
    {
        inserted = newSpace->addWindow(_window);
    }

    if (!inserted)
    {
        const bool restored = restoreSplit(_targetSpace, newSpace, newSplit);
        EGO_ASSERT(restored);

        return false;
    }

    if (originSpace && originSpace->isEmpty())
    {
        const bool collapsed = collapseEmptySpace(originSpace);
        EGO_ASSERT(collapsed);
    }

    return true;
}

bool ego::gui::DockingArea::dockWindowToRoot(const WindowPointer& _window, DockingPlacement _placement, float _ratio)
{
    if (!_window || !m_root || !std::isfinite(_ratio))
    {
        return false;
    }

    switch (_placement)
    {
    case DockingPlacement::Left:
    case DockingPlacement::Right:
    case DockingPlacement::Top:
    case DockingPlacement::Bottom:
        break;

    default:
        return false;
    }

    bool hasOtherDockedWindow = false;
    for (const WindowPointer& window : getWindows())
    {
        hasOtherDockedWindow = hasOtherDockedWindow || (window && window.get() != _window.get() && !window->isCollapsed());
    }
    if (!hasOtherDockedWindow)
    {
        return false;
    }

    const DockingSpacePointer originSpace = findWindowSpace(_window);
    if (_window->getParent() && !originSpace)
    {
        return false;
    }

    const DockingNodePointer previousRoot = m_root;
    const DockingSpacePointer newSpace = createSpace();
    const DockingSplitPointer newSplit = createSplit(_placement, _ratio);
    if (!previousRoot || !newSpace || !newSplit || !splitNode(previousRoot, newSpace, newSplit, _placement))
    {
        return false;
    }

    bool inserted = false;
    if (originSpace)
    {
        const DockingTabPointer tab = originSpace->releaseTab(_window);
        inserted = tab && newSpace->insertTab(tab);
        if (!inserted && tab)
        {
            const bool restored = originSpace->insertTab(tab);
            EGO_ASSERT(restored);
        }
    }
    else
    {
        inserted = newSpace->addWindow(_window);
    }

    if (!inserted)
    {
        const bool restored = restoreSplit(previousRoot, newSpace, newSplit);
        EGO_ASSERT(restored);

        return false;
    }

    if (originSpace && originSpace->isEmpty())
    {
        const bool collapsed = collapseEmptySpace(originSpace);
        EGO_ASSERT(collapsed);
    }

    return true;
}

ego::gui::WindowPointer ego::gui::DockingArea::undockWindow(const WindowPointer& _window)
{
    const DockingSpacePointer space = findWindowSpace(_window);
    if (!space)
    {
        return nullptr;
    }

    const WindowPointer window = space->removeWindow(_window);
    if (!window)
    {
        return nullptr;
    }

    if (space->isEmpty())
    {
        const bool collapsed = collapseEmptySpace(space);
        EGO_ASSERT(collapsed);
    }

    return window;
}

void ego::gui::DockingArea::clearInteraction()
{
    if (m_root)
    {
        m_root->clearInteraction();
    }
}

ego::gui::DockingSpaceID ego::gui::DockingArea::prepareSpaceID()
{
    if (m_nextSpaceID == InvalidDockingSpaceID)
    {
        return InvalidDockingSpaceID;
    }

    return m_nextSpaceID++;
}

ego::gui::DockingSpacePointer ego::gui::DockingArea::createSpace()
{
    return DockingSpace::Create(prepareSpaceID());
}

ego::gui::DockingSplitPointer ego::gui::DockingArea::createSplit(DockingPlacement _placement, float _ratio) const
{
    if (!std::isfinite(_ratio) || _placement == DockingPlacement::Center)
    {
        return nullptr;
    }

    const bool payloadFirst = _placement == DockingPlacement::Left || _placement == DockingPlacement::Top;
    const bool horizontal = _placement == DockingPlacement::Left || _placement == DockingPlacement::Right;
    const DockingAxis axis = horizontal ? DockingAxis::Horizontal : DockingAxis::Vertical;
    const float ratio = payloadFirst ? _ratio : 1.0f - _ratio;

    return DockingSplit::Create(axis, (std::clamp)(ratio, 0.05f, 0.95f));
}

bool ego::gui::DockingArea::replaceNode(const DockingNodePointer& _node, const DockingNodePointer& _replacement)
{
    if (!_node || !_replacement)
    {
        return false;
    }

    const WidgetPointer parent = _node->getParent();
    if (parent && rtti::IsObjectBasedOn<DockingSplit>(*parent))
    {
        return ego::StaticPointerCast<DockingSplit>(parent)->replaceChildNode(_node, _replacement);
    }

    if (parent.get() != this || m_root.get() != _node.get() || !detachChild(_node))
    {
        return false;
    }

    if (!attachChild(_replacement))
    {
        const bool restored = attachChild(_node);
        EGO_ASSERT(restored);

        return false;
    }

    m_root = _replacement;
    notifyTreeChanged();

    return true;
}

bool ego::gui::DockingArea::splitNode(
    const DockingNodePointer& _node,
    const DockingSpacePointer& _payloadSpace,
    const DockingSplitPointer& _split,
    DockingPlacement _placement)
{
    if (!_node || !_payloadSpace || !_split || _payloadSpace->getParent() || _split->getParent() || _split->getFirst() || _split->getSecond())
    {
        return false;
    }

    const bool payloadFirst = _placement == DockingPlacement::Left || _placement == DockingPlacement::Top;
    if (!replaceNode(_node, _split))
    {
        return false;
    }

    const bool childrenSet = payloadFirst ? _split->setChildren(_payloadSpace, _node) : _split->setChildren(_node, _payloadSpace);
    if (!childrenSet)
    {
        const bool restored = replaceNode(_split, _node);
        EGO_ASSERT(restored);

        return false;
    }

    return true;
}

bool ego::gui::DockingArea::restoreSplit(const DockingNodePointer& _node, const DockingSpacePointer& _payloadSpace, const DockingSplitPointer& _split)
{
    if (!_node || !_payloadSpace || !_split)
    {
        return false;
    }

    const bool payloadFirst = _split->getFirst().get() == _payloadSpace.get() && _split->getSecond().get() == _node.get();
    const bool payloadSecond = _split->getFirst().get() == _node.get() && _split->getSecond().get() == _payloadSpace.get();
    if (!payloadFirst && !payloadSecond)
    {
        return false;
    }

    const DockingNodePointer sibling = _split->releaseSibling(_payloadSpace);
    if (sibling.get() != _node.get())
    {
        return false;
    }

    if (replaceNode(_split, _node))
    {
        return true;
    }

    const bool restored = payloadFirst ? _split->setChildren(_payloadSpace, _node) : _split->setChildren(_node, _payloadSpace);
    EGO_ASSERT(restored);

    return false;
}

bool ego::gui::DockingArea::collapseEmptySpace(const DockingSpacePointer& _space)
{
    if (!_space || !_space->isEmpty())
    {
        return false;
    }

    const WidgetPointer parent = _space->getParent();
    if (parent.get() == this)
    {
        return m_root.get() == _space.get();
    }

    if (!parent || !rtti::IsObjectBasedOn<DockingSplit>(*parent))
    {
        return false;
    }

    const DockingSplitPointer split = ego::StaticPointerCast<DockingSplit>(parent);
    const bool spaceFirst = split->getFirst().get() == _space.get();
    const DockingNodePointer sibling = split->releaseSibling(_space);
    if (!sibling)
    {
        return false;
    }

    if (replaceNode(split, sibling))
    {
        return true;
    }

    const bool restored = spaceFirst ? split->setChildren(_space, sibling) : split->setChildren(sibling, _space);
    EGO_ASSERT(restored);

    return false;
}

ego::gui::DockingSpacePointer ego::gui::DockingArea::findFirstSpace(const DockingNodePointer& _node) const
{
    if (!_node)
    {
        return nullptr;
    }

    if (rtti::IsObjectBasedOn<DockingSpace>(*_node))
    {
        return ego::StaticPointerCast<DockingSpace>(_node);
    }

    if (!rtti::IsObjectBasedOn<DockingSplit>(*_node))
    {
        return nullptr;
    }

    const DockingSplitPointer split = ego::StaticPointerCast<DockingSplit>(_node);
    const DockingSpacePointer firstSpace = findFirstSpace(split->getFirst());

    return firstSpace ? firstSpace : findFirstSpace(split->getSecond());
}

ego::gui::DockingSpacePointer ego::gui::DockingArea::findSpace(const DockingNodePointer& _node, DockingSpaceID _spaceID) const
{
    if (!_node)
    {
        return nullptr;
    }

    if (rtti::IsObjectBasedOn<DockingSpace>(*_node))
    {
        const DockingSpacePointer space = ego::StaticPointerCast<DockingSpace>(_node);

        return space->getID() == _spaceID ? space : nullptr;
    }

    if (!rtti::IsObjectBasedOn<DockingSplit>(*_node))
    {
        return nullptr;
    }

    const DockingSplitPointer split = ego::StaticPointerCast<DockingSplit>(_node);
    const DockingSpacePointer firstSpace = findSpace(split->getFirst(), _spaceID);

    return firstSpace ? firstSpace : findSpace(split->getSecond(), _spaceID);
}

ego::gui::DockingSpacePointer ego::gui::DockingArea::findSpaceAt(const DockingNodePointer& _node, const Position& _position) const
{
    if (!_node || _node->getSpaceCount() == 0 || !_node->hitTest(_position))
    {
        return nullptr;
    }

    if (rtti::IsObjectBasedOn<DockingSpace>(*_node))
    {
        return ego::StaticPointerCast<DockingSpace>(_node);
    }

    if (!rtti::IsObjectBasedOn<DockingSplit>(*_node))
    {
        return nullptr;
    }

    const DockingSplitPointer split = ego::StaticPointerCast<DockingSplit>(_node);
    const DockingSpacePointer firstSpace = findSpaceAt(split->getFirst(), _position);

    return firstSpace ? firstSpace : findSpaceAt(split->getSecond(), _position);
}

ego::gui::DockingSpacePointer ego::gui::DockingArea::findWindowSpace(const DockingNodePointer& _node, const WindowPointer& _window) const
{
    if (!_node || !_window)
    {
        return nullptr;
    }

    if (rtti::IsObjectBasedOn<DockingSpace>(*_node))
    {
        const DockingSpacePointer space = ego::StaticPointerCast<DockingSpace>(_node);

        return space->findTab(_window) ? space : nullptr;
    }

    if (!rtti::IsObjectBasedOn<DockingSplit>(*_node))
    {
        return nullptr;
    }

    const DockingSplitPointer split = ego::StaticPointerCast<DockingSplit>(_node);
    const DockingSpacePointer firstSpace = findWindowSpace(split->getFirst(), _window);

    return firstSpace ? firstSpace : findWindowSpace(split->getSecond(), _window);
}

void ego::gui::DockingArea::appendWindows(const DockingNodePointer& _node, WindowCollection& _windows) const
{
    if (!_node)
    {
        return;
    }

    if (rtti::IsObjectBasedOn<DockingSpace>(*_node))
    {
        const DockingSpace::WindowCollection spaceWindows = ego::StaticPointerCast<DockingSpace>(_node)->getWindows();
        _windows.insert(_windows.end(), spaceWindows.begin(), spaceWindows.end());

        return;
    }

    if (!rtti::IsObjectBasedOn<DockingSplit>(*_node))
    {
        return;
    }

    const DockingSplitPointer split = ego::StaticPointerCast<DockingSplit>(_node);
    appendWindows(split->getFirst(), _windows);
    appendWindows(split->getSecond(), _windows);
}

ego::gui::DockingArea::DockingRequirements ego::gui::DockingArea::calculateRequirementsAfterRemoving(
    const DockingNodePointer& _node,
    const DockingSpace& _originSpace,
    const DockingSpace& _targetSpace,
    DockingAxis _payloadAxis,
    const Size& _minimumSpaceSize,
    float _separatorThickness) const
{
    if (!_node)
    {
        return DockingRequirements();
    }

    if (rtti::IsObjectBasedOn<DockingSpace>(*_node))
    {
        const DockingSpacePointer space = ego::StaticPointerCast<DockingSpace>(_node);
        if (space.get() == &_originSpace || space->getSpaceCount() == 0)
        {
            return DockingRequirements();
        }

        const Size minimumSpaceSize((std::max)(0.0f, _minimumSpaceSize.m_x), (std::max)(0.0f, _minimumSpaceSize.m_y));
        const DockingRequirements spaceRequirements{
            .m_spaceCount = 1,
            .m_minimumSize = minimumSpaceSize,
        };
        if (space.get() != &_targetSpace)
        {
            return spaceRequirements;
        }

        const DockingRequirements payloadRequirements{
            .m_spaceCount = 1,
            .m_minimumSize = minimumSpaceSize,
        };

        return CombineRequirements(spaceRequirements, payloadRequirements, _payloadAxis, _separatorThickness);
    }

    if (!rtti::IsObjectBasedOn<DockingSplit>(*_node))
    {
        return DockingRequirements();
    }

    const DockingSplitPointer split = ego::StaticPointerCast<DockingSplit>(_node);
    const DockingRequirements firstRequirements =
        calculateRequirementsAfterRemoving(split->getFirst(), _originSpace, _targetSpace, _payloadAxis, _minimumSpaceSize, _separatorThickness);
    const DockingRequirements secondRequirements =
        calculateRequirementsAfterRemoving(split->getSecond(), _originSpace, _targetSpace, _payloadAxis, _minimumSpaceSize, _separatorThickness);

    return CombineRequirements(firstRequirements, secondRequirements, split->getAxis(), _separatorThickness);
}

ego::gui::DockingArea::DockingRequirements ego::gui::DockingArea::CombineRequirements(
    const DockingRequirements& _first,
    const DockingRequirements& _second,
    DockingAxis _axis,
    float _separatorThickness)
{
    if (_first.m_spaceCount == 0)
    {
        return _second;
    }

    if (_second.m_spaceCount == 0)
    {
        return _first;
    }

    DockingRequirements combined;
    combined.m_spaceCount = _first.m_spaceCount + _second.m_spaceCount;
    const float separatorThickness = (std::max)(0.0f, _separatorThickness);
    if (_axis == DockingAxis::Horizontal)
    {
        combined.m_minimumSize =
            Size(_first.m_minimumSize.m_x + separatorThickness + _second.m_minimumSize.m_x, (std::max)(_first.m_minimumSize.m_y, _second.m_minimumSize.m_y));
    }
    else
    {
        combined.m_minimumSize =
            Size((std::max)(_first.m_minimumSize.m_x, _second.m_minimumSize.m_x), _first.m_minimumSize.m_y + separatorThickness + _second.m_minimumSize.m_y);
    }

    return combined;
}

ego::gui::Size ego::gui::DockingArea::calculatePreferredSize(const LayoutContext& _context, const LayoutConstraints& _constraints)
{
    if (m_root)
    {
        const LayoutConstraints constraints(m_root->getSpaceCount() > 0 ? _constraints.m_maximumSize : SizeZero);
        m_root->updatePreferredSize(_context, constraints);
    }

    return _constraints.m_maximumSize;
}

void ego::gui::DockingArea::updateGeometry(const LayoutContext& _context)
{
    if (m_root)
    {
        const Rect& bounds = getLayoutBounds();
        const Rect rootBounds = m_root->getSpaceCount() > 0 ? bounds : Rect(bounds.m_position, SizeZero);
        m_root->applyLayout(_context, rootBounds);
    }
}

size_t ego::gui::DockingArea::getChildCount() const
{
    return m_root ? 1 : 0;
}

ego::gui::WidgetPointer ego::gui::DockingArea::getChild(size_t) const
{
    return m_root;
}

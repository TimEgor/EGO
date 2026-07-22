#include "DockingLayout.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <utility>

#include "EgoCore/Assert/Assert.h"

bool ego::gui::DockingLayout::IsSpace(const TreeNode& _node)
{
    return !_node.m_first;
}

ego::gui::DockingLayout::TreeNodeReference ego::gui::DockingLayout::CreateSpace(uint32_t _id)
{
    TreeNodeReference node = new TreeNode();
    node->m_id = _id;

    return node;
}

void ego::gui::DockingLayout::ConvertToSplit(
    TreeNode& _node,
    uint32_t _splitID,
    DockingSplitAxis _axis,
    float _ratio,
    TreeNodeReference _first,
    TreeNodeReference _second)
{
    EGO_ASSERT(IsSpace(_node));
    EGO_ASSERT(_first && _second);
    EGO_ASSERT(_node.m_windows.empty());

    _node.m_id = _splitID;
    _node.m_selectedWindow.reset();
    _node.m_axis = _axis;
    _node.m_ratio = _ratio;
    _node.m_first = std::move(_first);
    _node.m_second = std::move(_second);
}

ego::gui::DockingSpaceID ego::gui::DockingLayout::GetFirstSpaceID(const TreeNode& _node)
{
    return IsSpace(_node) ? _node.m_id : GetFirstSpaceID(*_node.m_first);
}

ego::gui::DockingSpaceID ego::gui::DockingLayout::FindWindowSpaceID(const TreeNode& _node, const WindowPointer& _window)
{
    if (IsSpace(_node))
    {
        for (const WindowWeakPointer& weakWindow : _node.m_windows)
        {
            if (weakWindow.lock().get() == _window.get())
            {
                return _node.m_id;
            }
        }

        return InvalidDockingSpaceID;
    }

    const DockingSpaceID spaceID = FindWindowSpaceID(*_node.m_first, _window);

    return spaceID != InvalidDockingSpaceID ? spaceID : FindWindowSpaceID(*_node.m_second, _window);
}

ego::gui::DockingLayout::TreeNodeReference ego::gui::DockingLayout::FindWindowSpace(const TreeNodeReference& _node, const WindowPointer& _window)
{
    if (IsSpace(*_node))
    {
        for (const WindowWeakPointer& weakWindow : _node->m_windows)
        {
            if (weakWindow.lock().get() == _window.get())
            {
                return _node;
            }
        }

        return nullptr;
    }

    TreeNodeReference space = FindWindowSpace(_node->m_first, _window);

    return space ? space : FindWindowSpace(_node->m_second, _window);
}

ego::gui::DockingLayout::TreeNodeReference ego::gui::DockingLayout::ExtractNode(TreeNode& _node)
{
    TreeNodeReference extractedNode = new TreeNode();
    extractedNode->m_id = _node.m_id;
    extractedNode->m_windows = std::move(_node.m_windows);
    extractedNode->m_selectedWindow = _node.m_selectedWindow;
    extractedNode->m_axis = _node.m_axis;
    extractedNode->m_ratio = _node.m_ratio;
    extractedNode->m_first = std::move(_node.m_first);
    extractedNode->m_second = std::move(_node.m_second);

    _node.m_windows.clear();
    _node.m_selectedWindow.reset();
    _node.m_first.reset();
    _node.m_second.reset();

    return extractedNode;
}

bool ego::gui::DockingLayout::SetSplitRatio(TreeNode& _node, DockingSplitID _splitID, float _ratio)
{
    if (IsSpace(_node))
    {
        return false;
    }

    if (_node.m_id == _splitID)
    {
        _node.m_ratio = _ratio;
        return true;
    }

    return SetSplitRatio(*_node.m_first, _splitID, _ratio) || SetSplitRatio(*_node.m_second, _splitID, _ratio);
}

bool ego::gui::DockingLayout::HasWindowExcept(const TreeNode& _node, const WindowPointer& _window)
{
    if (IsSpace(_node))
    {
        for (const WindowWeakPointer& weakWindow : _node.m_windows)
        {
            const WindowPointer window = weakWindow.lock();
            if (window && window.get() != _window.get())
            {
                return true;
            }
        }

        return false;
    }

    return HasWindowExcept(*_node.m_first, _window) || HasWindowExcept(*_node.m_second, _window);
}

void ego::gui::DockingLayout::AddWindow(TreeNode& _node, const WindowPointer& _window)
{
    EGO_ASSERT(IsSpace(_node));
    _node.m_windows.push_back(_window);
    _node.m_selectedWindow = _window;
}

bool ego::gui::DockingLayout::RemoveWindow(TreeNode& _node, const WindowPointer& _window)
{
    EGO_ASSERT(IsSpace(_node));
    for (size_t windowIndex = 0; windowIndex < _node.m_windows.size(); ++windowIndex)
    {
        if (_node.m_windows[windowIndex].lock().get() != _window.get())
        {
            continue;
        }

        const WindowPointer selectedWindow = _node.m_selectedWindow.lock();
        const bool removedSelection = selectedWindow.get() == _window.get();
        _node.m_windows.erase(_node.m_windows.begin() + static_cast<std::ptrdiff_t>(windowIndex));
        if (removedSelection)
        {
            _node.m_selectedWindow = _node.m_windows.empty() ? WindowWeakPointer() : _node.m_windows.back();
        }

        return true;
    }

    return false;
}

bool ego::gui::DockingLayout::SelectWindow(TreeNode& _node, const WindowPointer& _window)
{
    EGO_ASSERT(IsSpace(_node));
    for (size_t windowIndex = 0; windowIndex < _node.m_windows.size(); ++windowIndex)
    {
        if (_node.m_windows[windowIndex].lock().get() == _window.get())
        {
            _node.m_selectedWindow = _window;
            return true;
        }
    }

    return false;
}

void ego::gui::DockingLayout::SplitNode(
    TreeNode& _node,
    const WindowPointer& _window,
    DockingPlacement _placement,
    float _splitRatio,
    uint32_t _newSpaceID,
    uint32_t _newSplitID)
{
    EGO_ASSERT(_placement != DockingPlacement::Center);

    const bool payloadFirst = _placement == DockingPlacement::Left || _placement == DockingPlacement::Top;
    const bool horizontal = _placement == DockingPlacement::Left || _placement == DockingPlacement::Right;
    const DockingSplitAxis axis = horizontal ? DockingSplitAxis::Horizontal : DockingSplitAxis::Vertical;
    const float ratio = payloadFirst ? (std::clamp)(_splitRatio, 0.05f, 0.95f) : (std::clamp)(1.0f - _splitRatio, 0.05f, 0.95f);

    TreeNodeReference existingNode = ExtractNode(_node);
    TreeNodeReference newSpace = CreateSpace(_newSpaceID);
    AddWindow(*newSpace, _window);
    if (payloadFirst)
    {
        ConvertToSplit(_node, _newSplitID, axis, ratio, std::move(newSpace), std::move(existingNode));
    }
    else
    {
        ConvertToSplit(_node, _newSplitID, axis, ratio, std::move(existingNode), std::move(newSpace));
    }
}

ego::gui::Size ego::gui::DockingLayout::ArrangeNode(
    const TreeNode& _node,
    const Rect& _bounds,
    const Size& _minimumSpaceSize,
    float _separatorThickness,
    Arrangement& _arrangement)
{
    if (IsSpace(_node))
    {
        Space space;
        space.m_id = _node.m_id;
        space.m_bounds = _bounds;
        for (const WindowWeakPointer& weakWindow : _node.m_windows)
        {
            const WindowPointer window = weakWindow.lock();
            if (window)
            {
                space.m_windows.push_back(window);
            }
        }

        space.m_selectedWindow = _node.m_selectedWindow.lock();

        _arrangement.m_spaces.push_back(std::move(space));

        return _minimumSpaceSize;
    }

    EGO_ASSERT(_node.m_first && _node.m_second);
    const bool horizontal = _node.m_axis == DockingSplitAxis::Horizontal;
    const float totalExtent = horizontal ? _bounds.m_size.m_x : _bounds.m_size.m_y;
    const float separatorThickness = (std::min)(_separatorThickness, totalExtent);
    const float availableExtent = (std::max)(0.0f, totalExtent - separatorThickness);
    const float firstExtent = std::floor(availableExtent * (std::clamp)(_node.m_ratio, 0.0f, 1.0f));
    const float secondExtent = availableExtent - firstExtent;

    Rect firstBounds = _bounds;
    Rect secondBounds = _bounds;
    Rect separatorBounds;
    if (horizontal)
    {
        firstBounds.m_size.m_x = firstExtent;
        secondBounds.m_position.m_x += firstExtent + separatorThickness;
        secondBounds.m_size.m_x = secondExtent;
        separatorBounds = Rect(_bounds.m_position.m_x + firstExtent, _bounds.m_position.m_y, separatorThickness, _bounds.m_size.m_y);
    }
    else
    {
        firstBounds.m_size.m_y = firstExtent;
        secondBounds.m_position.m_y += firstExtent + separatorThickness;
        secondBounds.m_size.m_y = secondExtent;
        separatorBounds = Rect(_bounds.m_position.m_x, _bounds.m_position.m_y + firstExtent, _bounds.m_size.m_x, separatorThickness);
    }

    const Size firstMinimumSize = ArrangeNode(*_node.m_first, firstBounds, _minimumSpaceSize, _separatorThickness, _arrangement);
    const Size secondMinimumSize = ArrangeNode(*_node.m_second, secondBounds, _minimumSpaceSize, _separatorThickness, _arrangement);
    const float crossExtent = horizontal ? separatorBounds.m_size.m_y : separatorBounds.m_size.m_x;
    if (crossExtent > 0.0f)
    {
        _arrangement.m_separators.push_back(
            {
                .m_splitID = _node.m_id,
                .m_axis = _node.m_axis,
                .m_bounds = separatorBounds,
                .m_splitBounds = _bounds,
                .m_firstMinimumExtent = horizontal ? firstMinimumSize.m_x : firstMinimumSize.m_y,
                .m_secondMinimumExtent = horizontal ? secondMinimumSize.m_x : secondMinimumSize.m_y,
            });
    }

    if (horizontal)
    {
        return Size(firstMinimumSize.m_x + _separatorThickness + secondMinimumSize.m_x, (std::max)(firstMinimumSize.m_y, secondMinimumSize.m_y));
    }

    return Size((std::max)(firstMinimumSize.m_x, secondMinimumSize.m_x), firstMinimumSize.m_y + _separatorThickness + secondMinimumSize.m_y);
}

ego::gui::DockingLayout::TreeNodeReference ego::gui::DockingLayout::FindSpace(const TreeNodeReference& _node, DockingSpaceID _spaceID)
{
    if (IsSpace(*_node))
    {
        return _node->m_id == _spaceID ? _node : nullptr;
    }

    TreeNodeReference space = FindSpace(_node->m_first, _spaceID);

    return space ? space : FindSpace(_node->m_second, _spaceID);
}

bool ego::gui::DockingLayout::CollapseEmptySpace(TreeNodeReference& _node, DockingSpaceID _spaceID)
{
    if (IsSpace(*_node))
    {
        return false;
    }

    EGO_ASSERT(_node->m_first && _node->m_second);
    if (IsSpace(*_node->m_first) && _node->m_first->m_id == _spaceID && _node->m_first->m_windows.empty())
    {
        _node = _node->m_second;
        return true;
    }

    if (IsSpace(*_node->m_second) && _node->m_second->m_id == _spaceID && _node->m_second->m_windows.empty())
    {
        _node = _node->m_first;
        return true;
    }

    return CollapseEmptySpace(_node->m_first, _spaceID) || CollapseEmptySpace(_node->m_second, _spaceID);
}

void ego::gui::DockingLayout::ValidateNode(const TreeNode& _node, std::vector<uint32_t>& _ids, WindowCollection& _windows)
{
    EGO_ASSERT(_node.m_id != 0);
    EGO_ASSERT(std::find(_ids.begin(), _ids.end(), _node.m_id) == _ids.end());
    _ids.push_back(_node.m_id);

    if (IsSpace(_node))
    {
        EGO_ASSERT(!_node.m_second);
        const WindowPointer selectedWindow = _node.m_selectedWindow.lock();
        EGO_ASSERT(!_node.m_windows.empty() || !selectedWindow);
        bool hasSelection = false;
        for (const WindowWeakPointer& weakWindow : _node.m_windows)
        {
            const WindowPointer window = weakWindow.lock();
            EGO_ASSERT(window);
            hasSelection = hasSelection || window.get() == selectedWindow.get();
            EGO_ASSERT(
                std::find_if(
                    _windows.begin(),
                    _windows.end(),
                    [&window](const WindowPointer& _currentWindow)
                    {
                        return _currentWindow.get() == window.get();
                    }) == _windows.end());
            _windows.push_back(window);
        }
        EGO_ASSERT(_node.m_windows.empty() || hasSelection);

        return;
    }

    EGO_ASSERT(_node.m_first && _node.m_second);
    EGO_ASSERT(_node.m_windows.empty());
    EGO_ASSERT(std::isfinite(_node.m_ratio) && _node.m_ratio >= 0.0f && _node.m_ratio <= 1.0f);
    ValidateNode(*_node.m_first, _ids, _windows);
    ValidateNode(*_node.m_second, _ids, _windows);
}

ego::gui::DockingLayout::DockingLayout()
{
    clear();
}

ego::gui::DockingLayout::~DockingLayout() = default;

void ego::gui::DockingLayout::clear()
{
    uint32_t rootID = prepareID();
    if (rootID == 0)
    {
        EGO_ASSERT_FAIL_MESSAGE("Docking node ID space is exhausted.");
        m_nextID = FirstID;
        rootID = prepareID();
    }

    m_root = CreateSpace(rootID);
    validate();
}

ego::gui::DockingLayout::Arrangement ego::gui::DockingLayout::arrange(const Rect& _bounds, const Size& _minimumSpaceSize, float _separatorThickness) const
{
    Arrangement arrangement;
    const Rect bounds(_bounds.m_position, Size((std::max)(0.0f, _bounds.m_size.m_x), (std::max)(0.0f, _bounds.m_size.m_y)));
    const Size minimumSpaceSize((std::max)(0.0f, _minimumSpaceSize.m_x), (std::max)(0.0f, _minimumSpaceSize.m_y));
    arrangement.m_minimumSize = ArrangeNode(*m_root, bounds, minimumSpaceSize, (std::max)(0.0f, _separatorThickness), arrangement);

    return arrangement;
}

ego::gui::DockingSpaceID ego::gui::DockingLayout::getDefaultSpaceID() const
{
    return GetFirstSpaceID(*m_root);
}

ego::gui::DockingSpaceID ego::gui::DockingLayout::findWindowSpace(const WindowPointer& _window) const
{
    if (!_window)
    {
        return InvalidDockingSpaceID;
    }

    return FindWindowSpaceID(*m_root, _window);
}

bool ego::gui::DockingLayout::isDocked(const WindowPointer& _window) const
{
    return findWindowSpace(_window) != InvalidDockingSpaceID;
}

bool ego::gui::DockingLayout::setSplitRatio(DockingSplitID _splitID, float _ratio)
{
    if (_splitID == InvalidDockingSplitID || !std::isfinite(_ratio))
    {
        return false;
    }

    return SetSplitRatio(*m_root, _splitID, (std::clamp)(_ratio, 0.0f, 1.0f));
}

bool ego::gui::DockingLayout::dockWindow(const WindowPointer& _window, DockingSpaceID _targetSpaceID, DockingPlacement _placement, float _splitRatio)
{
    if (!_window || _targetSpaceID == InvalidDockingSpaceID || !std::isfinite(_splitRatio))
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

    const TreeNodeReference targetSpace = FindSpace(m_root, _targetSpaceID);
    if (!targetSpace)
    {
        return false;
    }

    const TreeNodeReference originSpace = FindWindowSpace(m_root, _window);
    const bool targetsOrigin = originSpace && originSpace->m_id == targetSpace->m_id;
    if (targetsOrigin && _placement == DockingPlacement::Center)
    {
        return SelectWindow(*targetSpace, _window);
    }

    if (targetsOrigin && targetSpace->m_windows.size() == 1)
    {
        return false;
    }

    if (targetSpace->m_windows.empty())
    {
        _placement = DockingPlacement::Center;
    }

    uint32_t newSpaceID = 0;
    uint32_t newSplitID = 0;
    if (_placement != DockingPlacement::Center)
    {
        newSpaceID = prepareID();
        newSplitID = prepareID();
        if (newSpaceID == 0 || newSplitID == 0)
        {
            return false;
        }
    }

    const DockingSpaceID originSpaceID = originSpace ? originSpace->m_id : InvalidDockingSpaceID;
    if (originSpace)
    {
        const bool removed = RemoveWindow(*originSpace, _window);
        EGO_ASSERT(removed);
        if (!removed)
        {
            return false;
        }
    }

    if (_placement == DockingPlacement::Center)
    {
        AddWindow(*targetSpace, _window);
    }
    else
    {
        SplitNode(*targetSpace, _window, _placement, _splitRatio, newSpaceID, newSplitID);
    }

    if (originSpaceID != InvalidDockingSpaceID && originSpaceID != _targetSpaceID)
    {
        CollapseEmptySpace(m_root, originSpaceID);
    }

    validate();

    return true;
}

bool ego::gui::DockingLayout::dockWindowToRoot(const WindowPointer& _window, DockingPlacement _placement, float _splitRatio)
{
    if (!_window || _placement == DockingPlacement::Center || !std::isfinite(_splitRatio) || !HasWindowExcept(*m_root, _window))
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

    const uint32_t newSpaceID = prepareID();
    const uint32_t newSplitID = prepareID();
    if (newSpaceID == 0 || newSplitID == 0)
    {
        return false;
    }

    const TreeNodeReference originSpace = FindWindowSpace(m_root, _window);
    if (originSpace)
    {
        const DockingSpaceID originSpaceID = originSpace->m_id;
        const bool removed = RemoveWindow(*originSpace, _window);
        EGO_ASSERT(removed);
        if (!removed)
        {
            return false;
        }

        CollapseEmptySpace(m_root, originSpaceID);
    }

    SplitNode(*m_root, _window, _placement, _splitRatio, newSpaceID, newSplitID);
    validate();

    return true;
}

bool ego::gui::DockingLayout::undockWindow(const WindowPointer& _window)
{
    const TreeNodeReference space = _window ? FindWindowSpace(m_root, _window) : nullptr;
    if (!space)
    {
        return false;
    }

    const DockingSpaceID spaceID = space->m_id;
    const bool removed = RemoveWindow(*space, _window);
    EGO_ASSERT(removed);
    if (!removed)
    {
        return false;
    }

    CollapseEmptySpace(m_root, spaceID);
    validate();

    return true;
}

bool ego::gui::DockingLayout::selectWindow(const WindowPointer& _window)
{
    const TreeNodeReference space = _window ? FindWindowSpace(m_root, _window) : nullptr;

    return space && SelectWindow(*space, _window);
}

uint32_t ego::gui::DockingLayout::prepareID()
{
    if (m_nextID == 0)
    {
        return 0;
    }

    return m_nextID++;
}

void ego::gui::DockingLayout::validate() const
{
    std::vector<uint32_t> ids;
    WindowCollection windows;
    ValidateNode(*m_root, ids, windows);
}

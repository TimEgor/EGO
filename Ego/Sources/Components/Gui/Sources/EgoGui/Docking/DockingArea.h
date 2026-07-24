#pragma once

#include <cstddef>
#include <vector>

#include "EgoGui/Docking/DockingTypes.h"
#include "EgoGui/Widgets/Container.h"

namespace ego::gui
{
    class DockingArea;
    class DockingNode;
    class DockingSpace;
    class DockingSplit;
    class Window;

    EGO_POINTER(DockingArea);
    EGO_POINTER(DockingNode);
    EGO_POINTER(DockingSpace);
    EGO_POINTER(DockingSplit);
    EGO_POINTER(Window);

    class DockingArea final : public Container
    {
    public:
        using WindowCollection = std::vector<WindowPointer>;

        static DockingAreaPointer Create();

        bool initializeRoot();
        WindowCollection clearWindows();
        WindowCollection releaseWindows();

        DockingNodePointer getRootNode() const;
        DockingSpacePointer getDefaultSpace() const;
        DockingSpacePointer findSpace(DockingSpaceID _spaceID) const;
        DockingSpacePointer findSpaceAt(const Position& _position) const;
        DockingSpacePointer findWindowSpace(const WindowPointer& _window) const;
        Rect getDockingBounds(const DockingSpacePointer& _space) const;
        WindowCollection getWindows() const;
        bool isSplitAvailableAfterRemoving(
            const DockingSpacePointer& _originSpace,
            const DockingSpacePointer& _targetSpace,
            DockingAxis _axis,
            const Size& _rootSize,
            const Size& _minimumSpaceSize,
            float _separatorThickness) const;

        bool dockWindow(const WindowPointer& _window, const DockingSpacePointer& _targetSpace, DockingPlacement _placement, float _ratio);
        bool dockWindowToRoot(const WindowPointer& _window, DockingPlacement _placement, float _ratio);
        WindowPointer undockWindow(const WindowPointer& _window);
        void clearInteraction();

        EGO_RTTI_VIRTUAL(DockingArea, Container);

    protected:
        Size calculatePreferredSize(const LayoutContext& _context, const LayoutConstraints& _constraints) override;
        void updateGeometry(const LayoutContext& _context) override;

    private:
        struct DockingRequirements final
        {
            size_t m_spaceCount = 0;
            Size m_minimumSize = SizeZero;
        };

        DockingArea() = default;

        DockingSpaceID prepareSpaceID();
        DockingSpacePointer createSpace();
        DockingSplitPointer createSplit(DockingPlacement _placement, float _ratio) const;
        bool replaceNode(const DockingNodePointer& _node, const DockingNodePointer& _replacement);
        bool splitNode(
            const DockingNodePointer& _node,
            const DockingSpacePointer& _payloadSpace,
            const DockingSplitPointer& _split,
            DockingPlacement _placement);
        bool restoreSplit(const DockingNodePointer& _node, const DockingSpacePointer& _payloadSpace, const DockingSplitPointer& _split);
        bool collapseEmptySpace(const DockingSpacePointer& _space);

        DockingSpacePointer findFirstSpace(const DockingNodePointer& _node) const;
        DockingSpacePointer findSpace(const DockingNodePointer& _node, DockingSpaceID _spaceID) const;
        DockingSpacePointer findSpaceAt(const DockingNodePointer& _node, const Position& _position) const;
        DockingSpacePointer findWindowSpace(const DockingNodePointer& _node, const WindowPointer& _window) const;
        void appendWindows(const DockingNodePointer& _node, WindowCollection& _windows) const;
        DockingRequirements calculateRequirementsAfterRemoving(
            const DockingNodePointer& _node,
            const DockingSpace& _originSpace,
            const DockingSpace& _targetSpace,
            DockingAxis _payloadAxis,
            const Size& _minimumSpaceSize,
            float _separatorThickness) const;
        static DockingRequirements CombineRequirements(
            const DockingRequirements& _first,
            const DockingRequirements& _second,
            DockingAxis _axis,
            float _separatorThickness);

        size_t getChildCount() const override;
        WidgetPointer getChild(size_t _index) const override;

        static constexpr DockingSpaceID FirstSpaceID = 1;

        DockingNodePointer m_root = nullptr;
        DockingSpaceID m_nextSpaceID = FirstSpaceID;
    };
} // namespace ego::gui

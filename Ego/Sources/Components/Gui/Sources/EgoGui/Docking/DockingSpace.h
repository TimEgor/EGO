#pragma once

#include <cstddef>
#include <limits>
#include <vector>

#include "EgoGui/Docking/DockingNode.h"
#include "EgoGui/Docking/DockingTypes.h"

namespace ego::gui
{
    struct DockingStyle;
    class DockingSpace;
    class DockingTab;
    class Window;

    EGO_POINTER(DockingSpace);
    EGO_POINTER(DockingTab);
    EGO_POINTER(Window);

    class DockingSpace final : public DockingNode
    {
    public:
        using WindowCollection = std::vector<WindowPointer>;

        static DockingSpacePointer Create(DockingSpaceID _id);

        DockingSpaceID getID() const;
        WindowCollection getWindows() const;
        DockingTabPointer findTab(const WindowPointer& _window) const;

        bool addWindow(const WindowPointer& _window);
        WindowPointer removeWindow(const WindowPointer& _window);
        bool insertTab(const DockingTabPointer& _tab);
        DockingTabPointer releaseTab(const WindowPointer& _window);
        bool selectWindow(const WindowPointer& _window);
        bool selectTab(const DockingTab& _tab);
        bool isTabSelected(const DockingTab& _tab) const;
        bool hasNonCollapsedWindowExcept(const Window& _window) const;
        bool isEmpty() const;

        size_t getSpaceCount() const override;
        size_t getSpaceCountExcluding(const DockingSpace& _space) const override;
        Size getMinimumSize(const Size& _minimumSpaceSize, float _separatorThickness) const override;
        Size getMinimumSizeExcluding(const DockingSpace& _space, const Size& _minimumSpaceSize, float _separatorThickness) const override;
        void clearInteraction() override;
        bool isChildActive(size_t _index) const override;

        EGO_RTTI_VIRTUAL(DockingSpace, DockingNode);

    protected:
        Size calculatePreferredSize(const LayoutContext& _context, const LayoutConstraints& _constraints) override;
        void updateGeometry(const LayoutContext& _context) override;
        void drawBaseLayer(PaintContext& _context) const override;

    private:
        using TabCollection = std::vector<DockingTabPointer>;

        explicit DockingSpace(DockingSpaceID _id);

        size_t resolveSelectedTabIndex();
        void arrangeTabs(const Rect& _bounds, const DockingStyle& _style);
        Rect getContentBounds(const Rect& _bounds, const DockingStyle& _style) const;

        size_t getChildCount() const override;
        WidgetPointer getChild(size_t _index) const override;

        static constexpr size_t InvalidTabIndex = (std::numeric_limits<size_t>::max)();

        DockingSpaceID m_id;
        TabCollection m_tabs;
        size_t m_selectedTabIndex = InvalidTabIndex;
    };
} // namespace ego::gui

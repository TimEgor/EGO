#pragma once

#include <cstdint>
#include <vector>

#include "EgoCore/Reference/Pointer.h"
#include "EgoCore/Reference/Reference.h"

#include "EgoGui/Core/Geometry.h"

#include "DockingTypes.h"

namespace ego::gui
{
    class Window;
    EGO_POINTER(Window);
    EGO_WEAK_POINTER(Window);

    using DockingSplitID = uint32_t;
    inline constexpr DockingSplitID InvalidDockingSplitID = 0;

    enum class DockingSplitAxis
    {
        Horizontal,
        Vertical
    };

    struct DockingSeparator final
    {
        DockingSplitID m_splitID = InvalidDockingSplitID;
        DockingSplitAxis m_axis = DockingSplitAxis::Horizontal;
        Rect m_bounds;
        Rect m_splitBounds;
        float m_firstMinimumExtent = 0.0f;
        float m_secondMinimumExtent = 0.0f;
    };

    class DockingLayout final
    {
    public:
        using SeparatorCollection = std::vector<DockingSeparator>;
        using WindowCollection = std::vector<WindowPointer>;

        struct Space final
        {
            DockingSpaceID m_id = InvalidDockingSpaceID;
            Rect m_bounds;
            WindowCollection m_windows;
            WindowPointer m_selectedWindow;
        };

        using SpaceCollection = std::vector<Space>;

        struct Arrangement final
        {
            SpaceCollection m_spaces;
            SeparatorCollection m_separators;
            Size m_minimumSize;
        };

        DockingLayout();
        ~DockingLayout();

        DockingLayout(const DockingLayout&) = delete;
        DockingLayout& operator=(const DockingLayout&) = delete;

        void clear();
        Arrangement arrange(const Rect& _bounds, const Size& _minimumSpaceSize, float _separatorThickness) const;

        DockingSpaceID getDefaultSpaceID() const;
        DockingSpaceID findWindowSpace(const WindowPointer& _window) const;
        bool isDocked(const WindowPointer& _window) const;

        bool setSplitRatio(DockingSplitID _splitID, float _ratio);
        bool dockWindow(const WindowPointer& _window, DockingSpaceID _targetSpaceID, DockingPlacement _placement, float _splitRatio = 0.5f);
        bool dockWindowToRoot(const WindowPointer& _window, DockingPlacement _placement, float _splitRatio = 0.5f);
        bool undockWindow(const WindowPointer& _window);
        bool selectWindow(const WindowPointer& _window);

    private:
        struct TreeNode;
        EGO_REFERENCE(TreeNode);

        struct TreeNode final : public ego::STDDestroyMTCountable
        {
            uint32_t m_id = 0;
            std::vector<WindowWeakPointer> m_windows;
            WindowWeakPointer m_selectedWindow;
            DockingSplitAxis m_axis = DockingSplitAxis::Horizontal;
            float m_ratio = 0.5f;
            TreeNodeReference m_first;
            TreeNodeReference m_second;
        };

        static bool IsSpace(const TreeNode& _node);
        static TreeNodeReference CreateSpace(uint32_t _id);
        static void ConvertToSplit(
            TreeNode& _node,
            uint32_t _splitID,
            DockingSplitAxis _axis,
            float _ratio,
            TreeNodeReference _first,
            TreeNodeReference _second);
        static DockingSpaceID GetFirstSpaceID(const TreeNode& _node);
        static DockingSpaceID FindWindowSpaceID(const TreeNode& _node, const WindowPointer& _window);
        static TreeNodeReference FindWindowSpace(const TreeNodeReference& _node, const WindowPointer& _window);
        static TreeNodeReference FindSpace(const TreeNodeReference& _node, DockingSpaceID _spaceID);
        static TreeNodeReference ExtractNode(TreeNode& _node);
        static bool SetSplitRatio(TreeNode& _node, DockingSplitID _splitID, float _ratio);
        static bool HasWindowExcept(const TreeNode& _node, const WindowPointer& _window);
        static void AddWindow(TreeNode& _node, const WindowPointer& _window);
        static bool RemoveWindow(TreeNode& _node, const WindowPointer& _window);
        static bool SelectWindow(TreeNode& _node, const WindowPointer& _window);
        static void SplitNode(
            TreeNode& _node,
            const WindowPointer& _window,
            DockingPlacement _placement,
            float _splitRatio,
            uint32_t _newSpaceID,
            uint32_t _newSplitID);
        static Size ArrangeNode(
            const TreeNode& _node,
            const Rect& _bounds,
            const Size& _minimumSpaceSize,
            float _separatorThickness,
            Arrangement& _arrangement);
        static bool CollapseEmptySpace(TreeNodeReference& _node, DockingSpaceID _spaceID);
        static void ValidateNode(const TreeNode& _node, std::vector<uint32_t>& _ids, WindowCollection& _windows);

        uint32_t prepareID();
        void validate() const;

        static constexpr uint32_t FirstID = 1;

        TreeNodeReference m_root;
        uint32_t m_nextID = FirstID;
    };
} // namespace ego::gui

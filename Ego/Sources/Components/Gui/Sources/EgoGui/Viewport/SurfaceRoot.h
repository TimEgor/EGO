#pragma once

#include <vector>

#include "EgoCore/Patterns/NonCopyable.h"
#include "EgoCore/Patterns/NonInstanceable.h"

#include "EgoGui/Widgets/Container.h"

namespace ego::gui
{
    class SurfaceRoot;
    class WindowHost;

    EGO_POINTER(SurfaceRoot);
    EGO_WEAK_POINTER(SurfaceRoot);
    EGO_POINTER(WindowHost);

    class SurfaceRoot final
        : public Container
    {
    public:
        class SurfaceRootAccessor final
            : public NonInstanceable
        {
        public:
            static WindowHostPointer GetWindowHost(const SurfaceRoot& _root);
        };

        class TraversalScope final
            : public NonCopyable
        {
        public:
            explicit TraversalScope(SurfaceRoot& _root);
            ~TraversalScope() override;

        private:
            SurfaceRoot& m_root;
        };

        using WidgetCollection = std::vector<WidgetPointer>;

        ~SurfaceRoot() override;

        static SurfaceRootPointer Create();

        bool addWidget(const WidgetPointer& _widget);
        WidgetPointer removeWidget(const WidgetPointer& _widget);
        void clearWidgets();
        void bringWidgetToFront(const WidgetPointer& _widget);

        const WidgetCollection& getWidgets() const;
        WidgetPointer findWidgetAt(const Position& _position) const;
        bool isInputTarget(const WidgetPointer& _widget) const;

        void invalidateLayout();
        bool isLayoutInvalidated() const;
        bool updateLayoutIfNeeded(const LayoutContext& _context, const Size& _size);
        bool canMutateTree() const;

        EGO_RTTI_VIRTUAL(SurfaceRoot, Container);

    protected:
        Size calculatePreferredSize(const LayoutContext& _context, const LayoutConstraints& _constraints) override;
        void updateGeometry(const LayoutContext& _context) override;

    private:
        SurfaceRoot() = default;

        WindowHostPointer getWindowHost() const;
        void beginTraversal();
        void endTraversal();

        size_t getChildCount() const override;
        const WidgetPointer& getChild(size_t _index) const override;

        WidgetPointer m_windowHost = nullptr;
        size_t m_traversalDepth = 0;
        bool m_layoutInvalidated = true;
    };
} // namespace ego::gui

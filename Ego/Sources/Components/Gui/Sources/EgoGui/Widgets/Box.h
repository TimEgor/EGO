#pragma once

#include <cstddef>
#include <vector>

#include "EgoGui/Widgets/Container.h"

namespace ego::gui
{
    enum class BoxCrossAlignment
    {
        Start,
        Center,
        End,
        Stretch
    };

    class BoxSlot final
    {
    public:
        BoxSlot() = default;

        static BoxSlot Content(const Margin& _padding = Margin(), BoxCrossAlignment _alignment = BoxCrossAlignment::Stretch);
        static BoxSlot Fill(float _weight = 1.0f, const Margin& _padding = Margin(), BoxCrossAlignment _alignment = BoxCrossAlignment::Stretch);

        const Margin& getPadding() const;
        BoxCrossAlignment getAlignment() const;
        float getFillWeight() const;

    private:
        Margin m_padding;
        BoxCrossAlignment m_alignment = BoxCrossAlignment::Stretch;
        float m_fillWeight = 0.0f;
    };

    struct BoxChild final
    {
        BoxChild() = default;
        BoxChild(const WidgetPointer& _widget);
        BoxChild(const WidgetPointer& _widget, const BoxSlot& _slot);

        template <typename TWidget>
        BoxChild(const ego::SharedPointer<TWidget>& _widget);

        template <typename TWidget>
        BoxChild(const ego::SharedPointer<TWidget>& _widget, const BoxSlot& _slot);

        WidgetPointer m_widget;
        BoxSlot m_slot;
    };

    class Box : public Container
    {
    public:
        ~Box() override;

        bool addChild(const WidgetPointer& _widget, const BoxSlot& _slot = BoxSlot());
        WidgetPointer removeChild(const WidgetPointer& _widget);
        void clearChildren();

        EGO_RTTI_VIRTUAL(Box, Container);

    protected:
        enum class Orientation
        {
            Horizontal,
            Vertical
        };

        Box(Orientation _orientation, bool _useContentPadding);

    private:
        using ChildCollection = std::vector<BoxChild>;

        Size calculatePreferredSize(const LayoutContext& _context, const LayoutConstraints& _constraints) override;
        void updateGeometry(const LayoutContext& _context) override;

        size_t getChildCount() const override;
        const WidgetPointer& getChild(size_t _index) const override;

        ChildCollection m_children;
        Orientation m_orientation;
        bool m_useContentPadding = false;
    };

    BoxChild Fill(const WidgetPointer& _widget, float _weight = 1.0f);
} // namespace ego::gui

#include "Box.hpp"

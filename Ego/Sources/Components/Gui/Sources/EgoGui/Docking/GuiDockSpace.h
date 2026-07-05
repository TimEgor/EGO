#pragma once

#include <string>
#include <vector>

#include "EgoGui/GuiWidget.h"

namespace ego::gui
{
    using GuiDockTabID = uint32_t;
    inline constexpr GuiDockTabID InvalidGuiDockTabID = 0;

    class GuiDockSpace;
    EGO_POINTER(GuiDockSpace);

    struct GuiDockTabDesc final
    {
        GuiDockTabID m_id = InvalidGuiDockTabID;
        std::string m_title;
        GuiWidgetPointer m_content = nullptr;
    };

    struct GuiDockLayout final
    {
        GuiDockTabID m_activeTabID = InvalidGuiDockTabID;
    };

    class GuiDockSpace final : public GuiWidget
    {
    public:
        GuiDockSpace() = default;

        static GuiDockSpacePointer Create();

        bool openTab(const GuiDockTabDesc& _desc);
        bool closeTab(GuiDockTabID _tabID);
        bool setActiveTab(GuiDockTabID _tabID);

        GuiDockLayout saveLayout() const;
        bool restoreLayout(const GuiDockLayout& _layout);
        GuiReply handleEvent(const GuiInputEvent& _event) override;

        EGO_RTTI_VIRTUAL(GuiDockSpace, GuiWidget);

    protected:
        GuiSize onMeasure(const GuiLayoutContext& _context, const GuiSize& _availableSize) override;
        void onArrange(const GuiLayoutContext& _context, const GuiRect& _rect) override;
        void onPaint(GuiPaintContext& _context) const override;

    private:
        struct GuiDockTab final
        {
            GuiDockTabID m_id = InvalidGuiDockTabID;
            std::string m_title;
            GuiWidgetPointer m_content = nullptr;
        };

        using TabCollection = std::vector<GuiDockTab>;

        TabCollection::iterator findTab(GuiDockTabID _tabID);
        TabCollection::const_iterator findTab(GuiDockTabID _tabID) const;
        GuiDockTab* findTabAtPosition(const GuiPosition& _position);

        TabCollection m_tabs;
        GuiDockTabID m_activeTabID = InvalidGuiDockTabID;
        GuiDockTabID m_hoveredTabID = InvalidGuiDockTabID;
    };

} // namespace ego::gui

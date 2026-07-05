#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "EgoGui/GuiWidget.h"

namespace ego::gui
{
    class GuiRadioGroup;
    EGO_POINTER(GuiRadioGroup);

    class GuiRadioGroup final : public GuiWidget
    {
    public:
        using OptionIndex = size_t;

        GuiRadioGroup() = default;

        static GuiRadioGroupPointer Create();

        void setTitle(std::string _title);
        const std::string& getTitle() const;

        void addOption(std::string _text);
        void clearOptions();
        size_t getOptionCount() const;
        const std::string& getOptionText(OptionIndex _index) const;

        void setSelectedOptionIndex(OptionIndex _index);
        OptionIndex getSelectedOptionIndex() const;
        bool hasSelectedOption() const;
        const std::string& getSelectedOptionText() const;

        GuiReply handleEvent(const GuiInputEvent& _event) override;

        EGO_RTTI_VIRTUAL(GuiRadioGroup, GuiWidget);

    protected:
        GuiSize onMeasure(const GuiLayoutContext& _context, const GuiSize& _availableSize) override;
        void onArrange(const GuiLayoutContext& _context, const GuiRect& _rect) override;
        void onPaint(GuiPaintContext& _context) const override;

    private:
        static constexpr OptionIndex InvalidOptionIndex = static_cast<OptionIndex>(-1);

        OptionIndex findOptionAtPosition(const GuiPosition& _position) const;
        bool isValidOptionIndex(OptionIndex _index) const;

        std::string m_title;
        std::vector<std::string> m_options;
        std::vector<GuiRect> m_optionRects;
        GuiRect m_titleRect;
        OptionIndex m_selectedOptionIndex = InvalidOptionIndex;
        OptionIndex m_hoveredOptionIndex = InvalidOptionIndex;
        OptionIndex m_pressedOptionIndex = InvalidOptionIndex;
    };
} // namespace ego::gui

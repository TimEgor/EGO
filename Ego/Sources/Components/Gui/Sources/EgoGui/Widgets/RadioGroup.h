#pragma once

#include <cstddef>
#include <functional>
#include <string>
#include <vector>

#include "EgoCore/Callback/StableCallback.h"

#include "EgoGui/Widgets/Widget.h"

namespace ego::gui
{
    class RadioGroup;
    EGO_POINTER(RadioGroup);

    class RadioGroup final : public Widget
    {
    public:
        using OptionIndex = uint32_t;
        using SelectionChangedHandler = std::function<void(OptionIndex)>;

        static constexpr OptionIndex InvalidOptionIndex = static_cast<OptionIndex>(-1);

        static RadioGroupPointer Create();
        static RadioGroupPointer Create(std::string _title, std::vector<std::string> _options, OptionIndex _selectedOptionIndex = 0, SelectionChangedHandler _onChanged = {});

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
        void onChange(SelectionChangedHandler _handler);

        EGO_RTTI_VIRTUAL(RadioGroup, Widget);

    protected:
        InputReply onPointerMove(const PointerMoveEvent& _event) override;
        InputReply onMouseButton(const MouseButtonEvent& _event) override;
        void onPointerEnter(const Position& _position, const InputModifiers& _modifiers) override;
        void onPointerLeave(const Position& _position, const InputModifiers& _modifiers) override;
        void onPointerCaptureLost(const Position& _position) override;
        Size calculatePreferredSize(const LayoutContext& _context, const LayoutConstraints& _constraints) override;
        void updateGeometry(const LayoutContext& _context) override;
        void drawBaseLayer(PaintContext& _context) const override;

    private:
        RadioGroup() = default;

        OptionIndex findOptionAtPosition(const Position& _position) const;
        bool isValidOptionIndex(OptionIndex _index) const;
        void applyUserSelection(OptionIndex _index);

        std::string m_title;
        std::vector<std::string> m_options;
        std::vector<Rect> m_optionRects;
        Rect m_titleRect;
        OptionIndex m_selectedOptionIndex = InvalidOptionIndex;
        OptionIndex m_hoveredOptionIndex = InvalidOptionIndex;
        OptionIndex m_pressedOptionIndex = InvalidOptionIndex;
        StableCallback<OptionIndex> m_onSelectionChanged;
    };
} // namespace ego::gui

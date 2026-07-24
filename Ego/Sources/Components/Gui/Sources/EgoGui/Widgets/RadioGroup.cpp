#include "RadioGroup.h"

#include <algorithm>
#include <utility>

#include "EgoGui/Rendering/FontAtlas.h"
#include "EgoGui/Theme/Theme.h"

namespace
{
    const std::string EmptyString;
} // namespace

ego::gui::RadioGroupPointer ego::gui::RadioGroup::Create()
{
    return new RadioGroup();
}

ego::gui::RadioGroupPointer ego::gui::RadioGroup::Create(
    std::string _title,
    std::vector<std::string> _options,
    OptionIndex _selectedOptionIndex,
    SelectionChangedHandler _onChanged)
{
    const RadioGroupPointer radioGroup = new RadioGroup();
    radioGroup->setTitle(std::move(_title));
    for (std::string& option : _options)
    {
        radioGroup->addOption(std::move(option));
    }
    radioGroup->setSelectedOptionIndex(_selectedOptionIndex);
    radioGroup->onChange(std::move(_onChanged));

    return radioGroup;
}

void ego::gui::RadioGroup::setTitle(std::string _title)
{
    if (m_title == _title)
    {
        return;
    }

    m_title = std::move(_title);
    invalidateLayout();
}

const std::string& ego::gui::RadioGroup::getTitle() const
{
    return m_title;
}

void ego::gui::RadioGroup::addOption(std::string _text)
{
    m_options.push_back(std::move(_text));
    m_optionRects.resize(m_options.size());
    invalidateLayout();

    if (!hasSelectedOption())
    {
        m_selectedOptionIndex = 0;
    }
}

void ego::gui::RadioGroup::clearOptions()
{
    if (m_options.empty())
    {
        return;
    }

    m_options.clear();
    m_optionRects.clear();
    m_selectedOptionIndex = InvalidOptionIndex;
    m_hoveredOptionIndex = InvalidOptionIndex;
    m_pressedOptionIndex = InvalidOptionIndex;
    invalidateLayout();
}

size_t ego::gui::RadioGroup::getOptionCount() const
{
    return m_options.size();
}

const std::string& ego::gui::RadioGroup::getOptionText(OptionIndex _index) const
{
    return isValidOptionIndex(_index) ? m_options[_index] : EmptyString;
}

void ego::gui::RadioGroup::setSelectedOptionIndex(OptionIndex _index)
{
    const OptionIndex selectedOptionIndex = isValidOptionIndex(_index) ? _index : InvalidOptionIndex;
    if (m_selectedOptionIndex == selectedOptionIndex)
    {
        return;
    }

    m_selectedOptionIndex = selectedOptionIndex;
}

ego::gui::RadioGroup::OptionIndex ego::gui::RadioGroup::getSelectedOptionIndex() const
{
    return m_selectedOptionIndex;
}

bool ego::gui::RadioGroup::hasSelectedOption() const
{
    return isValidOptionIndex(m_selectedOptionIndex);
}

const std::string& ego::gui::RadioGroup::getSelectedOptionText() const
{
    return hasSelectedOption() ? m_options[m_selectedOptionIndex] : EmptyString;
}

void ego::gui::RadioGroup::onChange(SelectionChangedHandler _handler)
{
    m_onSelectionChanged.set(std::move(_handler));
}

void ego::gui::RadioGroup::applyUserSelection(OptionIndex _index)
{
    const OptionIndex selectedOptionIndex = isValidOptionIndex(_index) ? _index : InvalidOptionIndex;
    if (m_selectedOptionIndex == selectedOptionIndex)
    {
        return;
    }

    m_selectedOptionIndex = selectedOptionIndex;
    m_onSelectionChanged.invoke(m_selectedOptionIndex);
}

ego::gui::InputReply ego::gui::RadioGroup::onPointerMove(WidgetUpdateContext&, const PointerMoveEvent& _event)
{
    m_hoveredOptionIndex = findOptionAtPosition(_event.m_position);
    return InputReply::Unhandled;
}

ego::gui::InputReply ego::gui::RadioGroup::onMouseButton(WidgetUpdateContext&, const MouseButtonEvent& _event)
{
    const OptionIndex optionIndex = findOptionAtPosition(_event.m_position);
    if (_event.m_action == InputButtonAction::Pressed && _event.m_key == MouseInputKey::ButtonLeft && isValidOptionIndex(optionIndex) &&
        !isValidOptionIndex(m_pressedOptionIndex))
    {
        m_hoveredOptionIndex = optionIndex;
        m_pressedOptionIndex = optionIndex;
        return InputReply::FocusAndCapture;
    }

    if (_event.m_action == InputButtonAction::Released && _event.m_key == MouseInputKey::ButtonLeft && isValidOptionIndex(m_pressedOptionIndex))
    {
        const OptionIndex pressedOptionIndex = m_pressedOptionIndex;
        m_hoveredOptionIndex = optionIndex;
        m_pressedOptionIndex = InvalidOptionIndex;
        if (optionIndex == pressedOptionIndex)
        {
            applyUserSelection(optionIndex);
        }
        return InputReply::Handled;
    }

    return InputReply::Unhandled;
}

void ego::gui::RadioGroup::onPointerEnter(WidgetUpdateContext&, const Position& _position, const InputModifiers&)
{
    m_hoveredOptionIndex = findOptionAtPosition(_position);
}

void ego::gui::RadioGroup::onPointerLeave(WidgetUpdateContext&, const Position&, const InputModifiers&)
{
    m_hoveredOptionIndex = InvalidOptionIndex;
}

void ego::gui::RadioGroup::onPointerCaptureLost(WidgetUpdateContext&, const Position&)
{
    m_hoveredOptionIndex = InvalidOptionIndex;
    m_pressedOptionIndex = InvalidOptionIndex;
}

ego::gui::Size ego::gui::RadioGroup::calculatePreferredSize(const LayoutContext& _context, const LayoutConstraints&)
{
    const SelectionStyle& selection = _context.getTheme().m_selection;
    const float indicatorSize = (std::max)(0.0f, selection.m_indicatorSize);
    const float minimumOptionHeight = (std::max)(indicatorSize, (std::max)(0.0f, selection.m_minimumHeight));
    const float textOffset = indicatorSize + (std::max)(0.0f, selection.m_labelSpacing);
    Size result = SizeZero;

    if (!m_title.empty())
    {
        const Size titleSize = _context.m_fontAtlas ? _context.m_fontAtlas->measureText(m_title) : SizeZero;
        result.m_x = (std::max)(result.m_x, titleSize.m_x);
        result.m_y += titleSize.m_y + (std::max)(0.0f, selection.m_groupTitleSpacing);
    }

    for (OptionIndex optionIndex = 0; optionIndex < m_options.size(); ++optionIndex)
    {
        const Size textSize = _context.m_fontAtlas ? _context.m_fontAtlas->measureText(m_options[optionIndex]) : SizeZero;
        result.m_x = (std::max)(result.m_x, textOffset + textSize.m_x);
        result.m_y += (std::max)(minimumOptionHeight, textSize.m_y);

        if (optionIndex + 1 < m_options.size())
        {
            result.m_y += (std::max)(0.0f, selection.m_optionSpacing);
        }
    }

    return result;
}

void ego::gui::RadioGroup::updateGeometry(const LayoutContext& _context)
{
    const Rect& layoutBounds = getLayoutBounds();
    const SelectionStyle& selection = _context.getTheme().m_selection;
    const float indicatorSize = (std::max)(0.0f, selection.m_indicatorSize);
    const float minimumOptionHeight = (std::max)(indicatorSize, (std::max)(0.0f, selection.m_minimumHeight));
    const float optionSpacing = (std::max)(0.0f, selection.m_optionSpacing);
    float currentY = layoutBounds.m_position.m_y;
    if (!m_title.empty())
    {
        const Size titleSize = _context.m_fontAtlas ? _context.m_fontAtlas->measureText(m_title) : SizeZero;
        m_titleRect = Rect(layoutBounds.m_position.m_x, currentY, layoutBounds.m_size.m_x, titleSize.m_y);
        currentY += titleSize.m_y + (std::max)(0.0f, selection.m_groupTitleSpacing);
    }
    else
    {
        m_titleRect = Rect(layoutBounds.m_position, SizeZero);
    }

    m_optionRects.resize(m_options.size());
    for (OptionIndex optionIndex = 0; optionIndex < m_options.size(); ++optionIndex)
    {
        const Size textSize = _context.m_fontAtlas ? _context.m_fontAtlas->measureText(m_options[optionIndex]) : SizeZero;
        const float optionHeight = (std::max)(minimumOptionHeight, textSize.m_y);
        m_optionRects[optionIndex] = Rect(layoutBounds.m_position.m_x, currentY, layoutBounds.m_size.m_x, optionHeight);
        currentY += optionHeight + optionSpacing;
    }
}

void ego::gui::RadioGroup::drawBaseLayer(PaintContext& _context) const
{
    const Theme& theme = _context.getTheme();
    const SelectionStyle& selection = theme.m_selection;
    const float indicatorSize = (std::max)(0.0f, selection.m_indicatorSize);
    const float indicatorRadius = indicatorSize * 0.5f;
    const float innerRadius = (std::min)(indicatorRadius, (std::max)(0.0f, selection.m_radioInnerRadius));
    const float markRadius = (std::min)(innerRadius, (std::max)(0.0f, selection.m_radioMarkRadius));
    const float textOffset = indicatorSize + (std::max)(0.0f, selection.m_labelSpacing);

    if (!m_title.empty())
    {
        _context.drawText(m_title, m_titleRect, theme.m_typography.m_primary);
    }

    for (OptionIndex optionIndex = 0; optionIndex < m_options.size(); ++optionIndex)
    {
        const Rect& optionRect = m_optionRects[optionIndex];
        const Position center(optionRect.m_position.m_x + indicatorRadius, optionRect.m_position.m_y + indicatorRadius);
        const NormalizedColorRGBA& outerColor = selection.m_indicator.resolve(optionIndex == m_hoveredOptionIndex, optionIndex == m_pressedOptionIndex);

        _context.drawCircle(center, indicatorRadius, outerColor);
        _context.drawCircle(center, innerRadius, theme.m_field.m_surface);

        if (optionIndex == m_selectedOptionIndex)
        {
            _context.drawCircle(center, markRadius, selection.m_accent);
        }

        const Rect textRect(optionRect.m_position.m_x + textOffset, optionRect.m_position.m_y, optionRect.m_size.m_x - textOffset, optionRect.m_size.m_y);
        _context.drawText(m_options[optionIndex], textRect, theme.m_typography.m_primary);
    }
}

ego::gui::RadioGroup::OptionIndex ego::gui::RadioGroup::findOptionAtPosition(const Position& _position) const
{
    for (OptionIndex optionIndex = 0; optionIndex < m_optionRects.size(); ++optionIndex)
    {
        if (m_optionRects[optionIndex].contains(_position))
        {
            return optionIndex;
        }
    }

    return InvalidOptionIndex;
}

bool ego::gui::RadioGroup::isValidOptionIndex(OptionIndex _index) const
{
    return _index < m_options.size();
}

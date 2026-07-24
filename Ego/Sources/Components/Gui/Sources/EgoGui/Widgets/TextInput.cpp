#include "TextInput.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <utility>

#include "EgoGui/Core/TextEncoding.h"
#include "EgoGui/Rendering/FontAtlas.h"
#include "EgoGui/Theme/Theme.h"

ego::gui::TextInput::TextInput()
{
    applyFieldStyle(Theme::GetDefault().m_field);
    rebuildTextIndices();
}

ego::gui::TextInputPointer ego::gui::TextInput::Create()
{
    return new TextInput();
}

ego::gui::TextInputPointer ego::gui::TextInput::Create(TextInputDesc _desc)
{
    const TextInputPointer textInput = new TextInput();
    textInput->setText(std::move(_desc.m_text));
    textInput->setPlaceholder(std::move(_desc.m_placeholder));
    textInput->onChange(std::move(_desc.m_onChanged));
    textInput->onCommit(std::move(_desc.m_onCommitted));
    return textInput;
}

void ego::gui::TextInput::setText(std::string _text)
{
    if (m_text == _text)
    {
        return;
    }

    m_text = std::move(_text);
    if (m_isFocused)
    {
        m_focusStartText = m_text;
    }
    rebuildTextIndices();
    m_caretIndex = m_caretByteOffsets.empty() ? 0 : m_caretByteOffsets.size() - 1;
    clearSelection();
    invalidateLayout();
}

const std::string& ego::gui::TextInput::getText() const
{
    return m_text;
}

void ego::gui::TextInput::onChange(TextChangedHandler _handler)
{
    m_onTextChanged.set(std::move(_handler));
}

void ego::gui::TextInput::onCommit(TextCommittedHandler _handler)
{
    m_onTextCommitted.set(std::move(_handler));
}

void ego::gui::TextInput::setPlaceholder(std::string _placeholder)
{
    if (m_placeholder == _placeholder)
    {
        return;
    }

    m_placeholder = std::move(_placeholder);
    invalidateLayout();
}

const std::string& ego::gui::TextInput::getPlaceholder() const
{
    return m_placeholder;
}

void ego::gui::TextInput::setFocused(bool _isFocused)
{
    m_isFocused = _isFocused;
    if (!m_isFocused && !m_isPressed)
    {
        clearSelection();
    }
}

bool ego::gui::TextInput::isFocused() const
{
    return m_isFocused;
}

ego::gui::InputReply ego::gui::TextInput::onPointerMove(WidgetUpdateContext&, const PointerMoveEvent& _event)
{
    const bool containsMouse = m_inputRect.contains(_event.m_position);
    m_isHovered = containsMouse;
    if (m_isSelecting)
    {
        setCaretIndex(getCaretIndexAtPosition(_event.m_position), true);
        return InputReply::Handled;
    }
    return InputReply::Unhandled;
}

ego::gui::InputReply ego::gui::TextInput::onMouseButton(WidgetUpdateContext&, const MouseButtonEvent& _event)
{
    const bool containsMouse = m_inputRect.contains(_event.m_position);
    if (_event.m_action == InputButtonAction::Pressed && _event.m_key == MouseInputKey::ButtonLeft && !m_isPressed)
    {
        m_isHovered = containsMouse;
        m_isPressed = containsMouse;
        m_isSelecting = containsMouse;
        if (containsMouse)
        {
            setCaretIndex(getCaretIndexAtPosition(_event.m_position), _event.m_modifiers.m_shift);
        }

        return containsMouse ? InputReply::FocusAndCapture : InputReply::Unhandled;
    }

    if (_event.m_action == InputButtonAction::Released && _event.m_key == MouseInputKey::ButtonLeft && m_isPressed)
    {
        m_isPressed = false;
        m_isSelecting = false;
        m_isHovered = containsMouse;
        if (!m_isFocused)
        {
            clearSelection();
        }
        return InputReply::Handled;
    }

    return InputReply::Unhandled;
}

ego::gui::InputReply ego::gui::TextInput::onKey(WidgetUpdateContext&, const KeyEvent& _event)
{
    if (_event.m_action == InputButtonAction::Pressed && m_isFocused)
    {
        if (_event.m_key == KeyboardInputKey::Enter || _event.m_key == KeyboardInputKey::NumpadEnter)
        {
            m_suppressFocusLostCommit = true;
            notifyTextCommitted(TextCommitReason::Enter);
            return InputReply::ClearFocus;
        }

        if (_event.m_key == KeyboardInputKey::Escape)
        {
            if (m_text != m_focusStartText)
            {
                setText(m_focusStartText);
                notifyTextChanged();
            }

            m_suppressFocusLostCommit = true;
            notifyTextCommitted(TextCommitReason::Cancel);
            return InputReply::ClearFocus;
        }

        if (!handleKeyDown(_event.m_key, _event.m_modifiers))
        {
            return InputReply::Unhandled;
        }

        return InputReply::Handled;
    }

    if (_event.m_action == InputButtonAction::Released && (_event.m_key == KeyboardInputKey::LeftShift || _event.m_key == KeyboardInputKey::RightShift ||
                                                              _event.m_key == KeyboardInputKey::LeftControl || _event.m_key == KeyboardInputKey::RightControl))
    {
        return m_isFocused ? InputReply::Handled : InputReply::Unhandled;
    }

    return InputReply::Unhandled;
}

ego::gui::InputReply ego::gui::TextInput::onTextInput(WidgetUpdateContext&, const TextInputEvent& _event)
{
    if (m_isFocused)
    {
        return handleTextInput(_event.m_codepoint) ? InputReply::Handled : InputReply::Unhandled;
    }

    return InputReply::Unhandled;
}

void ego::gui::TextInput::onPointerEnter(WidgetUpdateContext&, const Position& _position, const InputModifiers&)
{
    m_isHovered = m_inputRect.contains(_position);
}

void ego::gui::TextInput::onPointerLeave(WidgetUpdateContext&, const Position&, const InputModifiers&)
{
    m_isHovered = false;
}

void ego::gui::TextInput::onPointerCaptureLost(WidgetUpdateContext&, const Position&)
{
    m_isPressed = false;
    m_isSelecting = false;
    m_isHovered = false;
    if (!m_isFocused)
    {
        clearSelection();
    }
}

void ego::gui::TextInput::onFocusChanged(WidgetUpdateContext&, FocusChange _change)
{
    if (_change == FocusChange::Gained)
    {
        m_focusStartText = m_text;
        m_suppressFocusLostCommit = false;
        setFocused(true);
        return;
    }

    const bool wasFocused = m_isFocused;
    setFocused(false);
    if (!wasFocused)
    {
        return;
    }

    if (m_suppressFocusLostCommit)
    {
        m_suppressFocusLostCommit = false;
    }
    else
    {
        notifyTextCommitted(TextCommitReason::FocusLost);
    }
}

ego::gui::Size ego::gui::TextInput::calculatePreferredSize(const LayoutContext& _context, const LayoutConstraints&)
{
    const FieldStyle& style = _context.getTheme().m_field;
    applyFieldStyle(style);

    const std::string& displayText = m_text.empty() ? m_placeholder : m_text;
    Size textSize = SizeZero;
    if (_context.m_fontAtlas)
    {
        textSize = _context.m_fontAtlas->measureText(displayText);
    }

    const float minimumWidth = (std::max)(0.0f, style.m_minimumWidth);
    const float fieldBorderThickness = (std::min)(m_fieldBorderThickness, (std::max)(0.0f, style.m_height) * 0.5f);
    const float textWidth = textSize.m_x + m_fieldTextPadding.getHorizontal() + fieldBorderThickness * 2.0f;
    return Size((std::max)(minimumWidth, textWidth), (std::max)(0.0f, style.m_height));
}

void ego::gui::TextInput::updateGeometry(const LayoutContext& _context)
{
    const FieldStyle& style = _context.getTheme().m_field;
    applyFieldStyle(style);

    rebuildCaretOffsets(_context.m_fontAtlas);
    const Rect& layoutBounds = getLayoutBounds();
    m_inputRect = Rect(layoutBounds.m_position.m_x, layoutBounds.m_position.m_y, layoutBounds.m_size.m_x, (std::max)(0.0f, style.m_height));
    ensureCaretVisible();
}

void ego::gui::TextInput::drawBaseLayer(PaintContext& _context) const
{
    const Theme& theme = _context.getTheme();
    const FieldStyle& field = theme.m_field;

    NormalizedColorRGBA borderColor = field.m_borderNormal;
    if (m_isHovered)
    {
        borderColor = field.m_borderHovered;
    }
    if (m_isFocused)
    {
        borderColor = field.m_borderFocused;
    }

    _context.drawBox(m_inputRect, borderColor);

    const float fieldBorderThickness = getEffectiveFieldBorderThickness();
    const Rect contentRect(
        m_inputRect.m_position.m_x + fieldBorderThickness,
        m_inputRect.m_position.m_y + fieldBorderThickness,
        (std::max)(0.0f, m_inputRect.m_size.m_x - fieldBorderThickness * 2.0f),
        (std::max)(0.0f, m_inputRect.m_size.m_y - fieldBorderThickness * 2.0f));
    _context.drawBox(contentRect, field.m_surface);

    const Rect textRect = getTextRect();
    const float textWidth = m_caretOffsets.empty() ? 0.0f : m_caretOffsets.back();
    const Rect scrolledTextRect(
        textRect.m_position.m_x - m_textScrollOffset,
        textRect.m_position.m_y,
        (std::max)(textRect.m_size.m_x + m_textScrollOffset, textWidth),
        textRect.m_size.m_y);
    _context.pushClipRect(textRect);
    if (m_isFocused && hasSelection())
    {
        const float selectionLeft = scrolledTextRect.m_position.m_x + getCaretOffset(getSelectionBegin());
        const float selectionRight = scrolledTextRect.m_position.m_x + getCaretOffset(getSelectionEnd());
        const float clippedSelectionLeft = (std::max)(textRect.m_position.m_x, selectionLeft);
        const float clippedSelectionRight = (std::min)(textRect.getRight(), selectionRight);
        if (clippedSelectionRight > clippedSelectionLeft)
        {
            const Rect selectionRect(clippedSelectionLeft, textRect.m_position.m_y, clippedSelectionRight - clippedSelectionLeft, textRect.m_size.m_y);
            _context.drawBox(selectionRect, field.m_selection);
        }
    }

    const bool hasText = !m_text.empty();
    if (hasText || !m_isFocused)
    {
        const std::string& displayText = hasText ? m_text : m_placeholder;
        const NormalizedColorRGBA& textColor = hasText ? theme.m_typography.m_primary : theme.m_typography.m_muted;
        _context.drawText(displayText, scrolledTextRect, textColor);
    }

    if (m_isFocused)
    {
        const float caretThickness = (std::min)(m_caretThickness, textRect.m_size.m_x);
        const float maximumCaretX = textRect.getRight() - caretThickness;
        const float caretX = (std::min)(scrolledTextRect.m_position.m_x + getCaretOffset(m_caretIndex), maximumCaretX);
        const Rect caretRect(caretX, textRect.m_position.m_y, caretThickness, textRect.m_size.m_y);
        _context.drawBox(caretRect, theme.m_typography.m_primary);
    }
    _context.popClipRect();
}

void ego::gui::TextInput::applyFieldStyle(const FieldStyle& _style)
{
    m_fieldTextPadding = Margin(
        (std::max)(0.0f, _style.m_textPadding.m_left),
        (std::max)(0.0f, _style.m_textPadding.m_top),
        (std::max)(0.0f, _style.m_textPadding.m_right),
        (std::max)(0.0f, _style.m_textPadding.m_bottom));
    m_fieldBorderThickness = (std::max)(0.0f, _style.m_borderThickness);
    m_caretThickness = (std::max)(0.0f, _style.m_caretThickness);
}

float ego::gui::TextInput::getEffectiveFieldBorderThickness() const
{
    const float maximumThickness = (std::max)(0.0f, (std::min)(m_inputRect.m_size.m_x, m_inputRect.m_size.m_y) * 0.5f);
    return (std::min)(m_fieldBorderThickness, maximumThickness);
}

bool ego::gui::TextInput::handleKeyDown(KeyboardInputKey _key, const InputModifiers& _modifiers)
{
    if (_key == KeyboardInputKey::LeftShift || _key == KeyboardInputKey::RightShift || _key == KeyboardInputKey::LeftControl ||
        _key == KeyboardInputKey::RightControl)
    {
        return true;
    }

    if (_modifiers.m_control && _key == KeyboardInputKey::A)
    {
        selectAll();
        return true;
    }

    if (_key == KeyboardInputKey::Left)
    {
        moveCaretLeft(_modifiers.m_shift);
        return true;
    }

    if (_key == KeyboardInputKey::Right)
    {
        moveCaretRight(_modifiers.m_shift);
        return true;
    }

    if (_key == KeyboardInputKey::Home)
    {
        moveCaretToStart(_modifiers.m_shift);
        return true;
    }

    if (_key == KeyboardInputKey::End)
    {
        moveCaretToEnd(_modifiers.m_shift);
        return true;
    }

    if (_key == KeyboardInputKey::Backspace)
    {
        deletePreviousCharacter();
        return true;
    }

    if (_key == KeyboardInputKey::Delete)
    {
        deleteNextCharacter();
        return true;
    }

    if (_key == KeyboardInputKey::Tab || _modifiers.m_control)
    {
        return false;
    }

    return false;
}

bool ego::gui::TextInput::handleTextInput(uint32_t _codepoint)
{
    if (_codepoint < 32)
    {
        return false;
    }

    insertCodepoint(_codepoint);
    return true;
}

void ego::gui::TextInput::insertCodepoint(uint32_t _codepoint)
{
    deleteSelection();
    std::string encodedCodepoint;
    if (!AppendUtf8Codepoint(encodedCodepoint, _codepoint))
    {
        return;
    }

    m_text.insert(getCaretByteOffset(m_caretIndex), encodedCodepoint);
    rebuildTextIndices();
    setCaretIndex(m_caretIndex + 1, false);
    notifyTextChanged();
}

void ego::gui::TextInput::deletePreviousCharacter()
{
    if (deleteSelection())
    {
        notifyTextChanged();
        return;
    }

    if (m_caretIndex == 0)
    {
        return;
    }

    const TextIndex removedIndex = m_caretIndex - 1;
    const size_t beginByteOffset = getCaretByteOffset(removedIndex);
    const size_t endByteOffset = getCaretByteOffset(m_caretIndex);
    m_text.erase(beginByteOffset, endByteOffset - beginByteOffset);
    rebuildTextIndices();
    setCaretIndex(removedIndex, false);
    notifyTextChanged();
}

void ego::gui::TextInput::deleteNextCharacter()
{
    if (deleteSelection())
    {
        notifyTextChanged();
        return;
    }

    if (m_caretIndex + 1 >= m_caretByteOffsets.size())
    {
        return;
    }

    const size_t beginByteOffset = getCaretByteOffset(m_caretIndex);
    const size_t endByteOffset = getCaretByteOffset(m_caretIndex + 1);
    m_text.erase(beginByteOffset, endByteOffset - beginByteOffset);
    rebuildTextIndices();
    setCaretIndex(m_caretIndex, false);
    notifyTextChanged();
}

bool ego::gui::TextInput::deleteSelection()
{
    if (!hasSelection())
    {
        return false;
    }

    const TextIndex selectionBegin = getSelectionBegin();
    const TextIndex selectionEnd = getSelectionEnd();
    const size_t beginByteOffset = getCaretByteOffset(selectionBegin);
    const size_t endByteOffset = getCaretByteOffset(selectionEnd);
    m_text.erase(beginByteOffset, endByteOffset - beginByteOffset);
    rebuildTextIndices();
    setCaretIndex(selectionBegin, false);
    return true;
}

void ego::gui::TextInput::selectAll()
{
    if (!m_text.empty())
    {
        m_selectionAnchor = 0;
        m_caretIndex = m_caretByteOffsets.size() - 1;
    }
}

ego::gui::Rect ego::gui::TextInput::getTextRect() const
{
    const float fieldBorderThickness = getEffectiveFieldBorderThickness();
    const float contentWidth = (std::max)(0.0f, m_inputRect.m_size.m_x - fieldBorderThickness * 2.0f);
    const float contentHeight = (std::max)(0.0f, m_inputRect.m_size.m_y - fieldBorderThickness * 2.0f);
    const float leftPadding = (std::min)(m_fieldTextPadding.m_left, contentWidth);
    const float topPadding = (std::min)(m_fieldTextPadding.m_top, contentHeight);
    const float rightPadding = (std::min)(m_fieldTextPadding.m_right, contentWidth - leftPadding);
    const float bottomPadding = (std::min)(m_fieldTextPadding.m_bottom, contentHeight - topPadding);

    return Rect(
        m_inputRect.m_position.m_x + fieldBorderThickness + leftPadding,
        m_inputRect.m_position.m_y + fieldBorderThickness + topPadding,
        contentWidth - leftPadding - rightPadding,
        contentHeight - topPadding - bottomPadding);
}

void ego::gui::TextInput::rebuildCaretOffsets(const FontAtlasPointer& _fontAtlas)
{
    rebuildTextIndices();
    m_caretOffsets.clear();
    m_caretOffsets.reserve(m_caretByteOffsets.size());
    m_caretOffsets.push_back(0.0f);

    float currentOffset = 0.0f;
    if (_fontAtlas)
    {
        size_t byteOffset = 0;
        uint32_t codepoint = 0;
        while (DecodeNextUtf8(m_text, byteOffset, codepoint))
        {
            const FontGlyph* glyph = _fontAtlas->getGlyph(codepoint);
            if (glyph)
            {
                currentOffset += glyph->m_advance;
            }

            m_caretOffsets.push_back(currentOffset);
        }
    }
    else
    {
        m_caretOffsets.resize(m_caretByteOffsets.size(), 0.0f);
    }

    const TextIndex characterCount = m_caretByteOffsets.empty() ? 0 : m_caretByteOffsets.size() - 1;
    m_caretIndex = (std::min)(m_caretIndex, characterCount);
    m_selectionAnchor = (std::min)(m_selectionAnchor, characterCount);
}

void ego::gui::TextInput::rebuildTextIndices()
{
    m_caretByteOffsets.clear();
    m_caretByteOffsets.push_back(0);

    size_t byteOffset = 0;
    uint32_t codepoint = 0;
    while (DecodeNextUtf8(m_text, byteOffset, codepoint))
    {
        m_caretByteOffsets.push_back(byteOffset);
    }
}

void ego::gui::TextInput::ensureCaretVisible()
{
    const float textWidth = getTextRect().m_size.m_x;
    if (textWidth <= 0.0f)
    {
        m_textScrollOffset = 0.0f;
        return;
    }

    const float caretOffset = getCaretOffset(m_caretIndex);
    if (caretOffset < m_textScrollOffset)
    {
        m_textScrollOffset = caretOffset;
    }
    else if (caretOffset + m_caretThickness > m_textScrollOffset + textWidth)
    {
        m_textScrollOffset = caretOffset + m_caretThickness - textWidth;
    }

    const float maximumOffset = m_caretOffsets.empty() ? 0.0f : (std::max)(0.0f, m_caretOffsets.back() - textWidth);
    m_textScrollOffset = (std::clamp)(m_textScrollOffset, 0.0f, maximumOffset);
}

ego::gui::TextInput::TextIndex ego::gui::TextInput::getCaretIndexAtPosition(const Position& _position) const
{
    const Rect textRect = getTextRect();
    const float localX = (std::max)(0.0f, _position.m_x - textRect.m_position.m_x + m_textScrollOffset);
    for (TextIndex index = 0; index + 1 < m_caretOffsets.size(); ++index)
    {
        const float left = m_caretOffsets[index];
        const float right = m_caretOffsets[index + 1];
        const float center = left + (right - left) * 0.5f;
        if (localX < center)
        {
            return index;
        }
    }

    return m_caretByteOffsets.empty() ? 0 : m_caretByteOffsets.size() - 1;
}

float ego::gui::TextInput::getCaretOffset(TextIndex _index) const
{
    if (m_caretOffsets.empty())
    {
        return 0.0f;
    }

    const TextIndex clampedIndex = (std::min)(_index, m_caretOffsets.size() - 1);
    return m_caretOffsets[clampedIndex];
}

size_t ego::gui::TextInput::getCaretByteOffset(TextIndex _index) const
{
    if (m_caretByteOffsets.empty())
    {
        return 0;
    }

    return m_caretByteOffsets[(std::min)(_index, m_caretByteOffsets.size() - 1)];
}

ego::gui::TextInput::TextIndex ego::gui::TextInput::getSelectionBegin() const
{
    return (std::min)(m_caretIndex, m_selectionAnchor);
}

ego::gui::TextInput::TextIndex ego::gui::TextInput::getSelectionEnd() const
{
    return (std::max)(m_caretIndex, m_selectionAnchor);
}

bool ego::gui::TextInput::hasSelection() const
{
    return m_caretIndex != m_selectionAnchor;
}

void ego::gui::TextInput::clearSelection()
{
    m_selectionAnchor = m_caretIndex;
}

void ego::gui::TextInput::setCaretIndex(TextIndex _index, bool _keepSelection)
{
    const TextIndex characterCount = m_caretByteOffsets.empty() ? 0 : m_caretByteOffsets.size() - 1;
    m_caretIndex = (std::min)(_index, characterCount);
    if (!_keepSelection)
    {
        clearSelection();
    }
}

void ego::gui::TextInput::moveCaretLeft(bool _keepSelection)
{
    if (!_keepSelection && hasSelection())
    {
        setCaretIndex(getSelectionBegin(), false);
        return;
    }

    if (m_caretIndex > 0)
    {
        setCaretIndex(m_caretIndex - 1, _keepSelection);
    }
    else if (!_keepSelection)
    {
        clearSelection();
    }
}

void ego::gui::TextInput::moveCaretRight(bool _keepSelection)
{
    if (!_keepSelection && hasSelection())
    {
        setCaretIndex(getSelectionEnd(), false);
        return;
    }

    if (m_caretIndex + 1 < m_caretByteOffsets.size())
    {
        setCaretIndex(m_caretIndex + 1, _keepSelection);
    }
    else if (!_keepSelection)
    {
        clearSelection();
    }
}

void ego::gui::TextInput::moveCaretToStart(bool _keepSelection)
{
    setCaretIndex(0, _keepSelection);
}

void ego::gui::TextInput::moveCaretToEnd(bool _keepSelection)
{
    setCaretIndex(m_text.size(), _keepSelection);
}

void ego::gui::TextInput::notifyTextChanged()
{
    invalidateLayout();
    const std::string text = m_text;
    m_onTextChanged.invoke(text);
}

void ego::gui::TextInput::notifyTextCommitted(TextCommitReason _reason)
{
    const std::string text = m_text;
    m_onTextCommitted.invoke(text, _reason);
}

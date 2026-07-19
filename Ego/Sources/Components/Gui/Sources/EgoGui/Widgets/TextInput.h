#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

#include "EgoCore/Callback/StableCallback.h"

#include "EgoGui/Widgets/Widget.h"

namespace ego::gui
{
    struct FieldStyle;

    enum class TextCommitReason
    {
        Enter,
        FocusLost,
        Cancel
    };

    class TextInput;
    EGO_POINTER(TextInput);

    using TextChangedHandler = std::function<void(const std::string&)>;
    using TextCommittedHandler = std::function<void(const std::string&, TextCommitReason)>;

    struct TextInputDesc final
    {
        std::string m_text;
        std::string m_placeholder;
        TextChangedHandler m_onChanged;
        TextCommittedHandler m_onCommitted;
    };

    class TextInput final : public Widget
    {
    public:
        static TextInputPointer Create();
        static TextInputPointer Create(TextInputDesc _desc);

        void setText(std::string _text);
        const std::string& getText() const;
        void onChange(TextChangedHandler _handler);
        void onCommit(TextCommittedHandler _handler);

        void setPlaceholder(std::string _placeholder);
        const std::string& getPlaceholder() const;

        bool isFocused() const;

        EGO_RTTI_VIRTUAL(TextInput, Widget);

    protected:
        InputReply onPointerMove(const PointerMoveEvent& _event) override;
        InputReply onMouseButton(const MouseButtonEvent& _event) override;
        InputReply onKey(const KeyEvent& _event) override;
        InputReply onTextInput(const TextInputEvent& _event) override;
        void onPointerEnter(const Position& _position, const InputModifiers& _modifiers) override;
        void onPointerLeave(const Position& _position, const InputModifiers& _modifiers) override;
        void onPointerCaptureLost(const Position& _position) override;
        void onFocusChanged(FocusChange _change) override;
        Size calculatePreferredSize(const LayoutContext& _context, const LayoutConstraints& _constraints) override;
        void updateGeometry(const LayoutContext& _context) override;
        void drawBaseLayer(PaintContext& _context) const override;

    private:
        using TextIndex = size_t;

        TextInput();

        void applyFieldStyle(const FieldStyle& _style);
        float getEffectiveFieldBorderThickness() const;
        void setFocused(bool _isFocused);
        bool handleKeyDown(KeyboardInputKey _key, const InputModifiers& _modifiers);
        bool handleTextInput(uint32_t _codepoint);
        void insertCodepoint(uint32_t _codepoint);
        void deletePreviousCharacter();
        void deleteNextCharacter();
        bool deleteSelection();
        void selectAll();

        Rect getTextRect() const;
        void rebuildCaretOffsets(const FontAtlasPointer& _fontAtlas);
        void rebuildTextIndices();
        void ensureCaretVisible();
        TextIndex getCaretIndexAtPosition(const Position& _position) const;
        float getCaretOffset(TextIndex _index) const;
        size_t getCaretByteOffset(TextIndex _index) const;
        TextIndex getSelectionBegin() const;
        TextIndex getSelectionEnd() const;
        bool hasSelection() const;
        void clearSelection();
        void setCaretIndex(TextIndex _index, bool _keepSelection);
        void moveCaretLeft(bool _keepSelection);
        void moveCaretRight(bool _keepSelection);
        void moveCaretToStart(bool _keepSelection);
        void moveCaretToEnd(bool _keepSelection);
        void notifyTextChanged();
        void notifyTextCommitted(TextCommitReason _reason);

        std::string m_text;
        std::string m_placeholder;
        std::string m_focusStartText;
        Rect m_inputRect;
        std::vector<size_t> m_caretByteOffsets;
        std::vector<float> m_caretOffsets;
        TextIndex m_caretIndex = 0;
        TextIndex m_selectionAnchor = 0;
        Margin m_fieldTextPadding;
        float m_fieldBorderThickness = 0.0f;
        float m_caretThickness = 0.0f;
        float m_textScrollOffset = 0.0f;
        bool m_isFocused = false;
        bool m_isHovered = false;
        bool m_isPressed = false;
        bool m_isSelecting = false;
        bool m_suppressFocusLostCommit = false;
        StableCallback<const std::string&> m_onTextChanged;
        StableCallback<const std::string&, TextCommitReason> m_onTextCommitted;
    };
} // namespace ego::gui

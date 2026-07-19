#include "GuiDemo.h"

#include <cstdint>
#include <string>

#include "EgoCore/UtilsMacros.h"

#include "EgoEngine/EngineSession.h"

bool ego::demo::GuiDemo::init(const InitData& _initData)
{
    EGO_CHECK_INITIALIZATION(!_initData.m_engineSession.isExpired());
    EGO_CHECK_INITIALIZATION(m_viewport.isExpired());

    const engine::EngineSessionPointer engineSession = _initData.m_engineSession.lock();
    EGO_CHECK_INITIALIZATION(engineSession);

    const gui::GuiControllerPointer guiController = engineSession->getGuiControllerPointer();
    EGO_CHECK_INITIALIZATION(guiController);

    const gui::ViewportPointer primaryViewport = guiController->getPrimaryViewport();
    EGO_CHECK_INITIALIZATION(primaryViewport);

    m_window = createWindow();
    EGO_CHECK_INITIALIZATION(m_window);
    EGO_CHECK_INITIALIZATION(primaryViewport->add(m_window));
    m_viewport = primaryViewport;
    return true;
}

void ego::demo::GuiDemo::release()
{
    const gui::ViewportPointer viewport = m_viewport.lock();
    if (viewport && m_window)
    {
        viewport->remove(m_window);
    }

    m_window = nullptr;
    m_viewport.reset();
}

ego::gui::WindowPointer ego::demo::GuiDemo::createWindow() const
{
    const gui::Size initialWindowSize(400.0f, 300.0f);

    const gui::TextPointer resolution = gui::Text::Create();
    const gui::TextPointer textInputState = gui::Text::Create("Value: Directional Light");
    const gui::TextPointer textInputCommitState = gui::Text::Create("Commit: -");
    const gui::TextPointer checkBoxState = gui::Text::Create("Checked: true");
    const gui::TextPointer radioGroupState = gui::Text::Create("Selected index: 0");
    const gui::TextPointer buttonState = gui::Text::Create("Clicks: 0");
    const gui::TextPointer scrollState = gui::Text::Create("Scroll offset: 0");

    const gui::TextInputPointer textInput = gui::TextInput::Create(
        {
            .m_text = "Directional Light",
            .m_placeholder = "Enter a name",
            .m_onChanged =
                [textInputState](const std::string& _text)
            {
                textInputState->setText("Value: " + _text);
            },
            .m_onCommitted =
                [textInputCommitState](const std::string& _text, gui::TextCommitReason _reason)
            {
                std::string reason;
                switch (_reason)
                {
                case gui::TextCommitReason::Enter:
                    reason = "Enter";
                    break;
                case gui::TextCommitReason::FocusLost:
                    reason = "Focus lost";
                    break;
                case gui::TextCommitReason::Cancel:
                    reason = "Cancel";
                    break;
                }

                textInputCommitState->setText("Commit: " + reason + ", value: " + _text);
            },
        });

    const gui::CheckBoxPointer checkBox = gui::CheckBox::Create(
        "Enable shadows",
        true,
        [checkBoxState](bool _isChecked)
        {
            checkBoxState->setText(_isChecked ? "Checked: true" : "Checked: false");
        });

    const gui::RadioGroupPointer radioGroup = gui::RadioGroup::Create(
        "Render mode",
        {"Shaded", "Wireframe", "Lighting Only"},
        0,
        [radioGroupState](gui::RadioGroup::OptionIndex _optionIndex)
        {
            radioGroupState->setText("Selected index: " + std::to_string(_optionIndex));
        });

    const gui::ButtonPointer button = gui::Button::Create(
        "Increment counter",
        [buttonState, clickCount = uint32_t(0)]() mutable
        {
            ++clickCount;
            buttonState->setText("Clicks: " + std::to_string(clickCount));
        });
    const gui::RowPointer buttonRow = gui::Row::Create({gui::Fill(button)});
    const gui::ColumnPointer labeledTextInput = gui::Labeled("Name", textInput);
    const gui::VerticalPanelPointer panel = gui::VerticalPanel::Create(
        {
            resolution,
            labeledTextInput,
            textInputState,
            textInputCommitState,
            checkBox,
            checkBoxState,
            radioGroup,
            radioGroupState,
            buttonRow,
            buttonState,
            scrollState,
        });

    const gui::ScrollViewPointer scrollView = gui::ScrollView::Create(
        {
            .m_content = panel,
            .m_horizontalMode = gui::ScrollBarMode::Disabled,
            .m_verticalMode = gui::ScrollBarMode::Auto,
            .m_onChanged =
                [scrollState](const gui::Position& _offset)
            {
                scrollState->setText("Scroll offset: " + std::to_string(static_cast<uint32_t>(_offset.m_y)));
            },
        });

    const gui::WindowSizeChangedHandler updateResolution = [resolution](const gui::Size& _size)
    {
        resolution->setText("Window resolution: " + std::to_string(static_cast<uint32_t>(_size.m_x)) + " x " + std::to_string(static_cast<uint32_t>(_size.m_y)));
    };

    EGO_CHECK_RETURN_NULL(
        resolution && textInputState && textInputCommitState && checkBoxState && radioGroupState && buttonState && scrollState && textInput && checkBox && radioGroup && button &&
        buttonRow && labeledTextInput && panel && scrollView);

    const gui::WindowPointer window = gui::Window::Create(
        {
            .m_title = "EgoGui Demo",
            .m_bounds = gui::Rect(gui::Position(15.0f, 15.0f), initialWindowSize),
            .m_content = scrollView,
            .m_onSizeChanged = updateResolution,
        });
    EGO_CHECK_RETURN_NULL(window);

    updateResolution(window->getSize());
    return window;
}

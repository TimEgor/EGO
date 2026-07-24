#include "GuiDemo.h"

#include <cstdint>
#include <string>

#include "EgoCore/UtilsMacros.h"

#include "EgoEngine/EngineSession.h"
#include "EgoEngine/Graphic/SceneRender/Component/CameraComponent.h"
#include "EgoEngine/Graphic/SceneRender/Render.h"

namespace
{
    constexpr auto TriangleColor = ego::FloatVector4(0.1f, 0.8f, 1.0f, 1.0f);
    constexpr auto TriangleTop = ego::FloatVector3(0.0f, 0.6f, 0.0f);
    constexpr auto TriangleBottomLeft = ego::FloatVector3(-0.6f, -0.5f, 0.0f);
    constexpr auto TriangleBottomRight = ego::FloatVector3(0.6f, -0.5f, 0.0f);
} // namespace

bool ego::demo::GuiDemo::init(const InitData& _initData)
{
    EGO_CHECK_INITIALIZATION(!_initData.m_engineSession.isExpired());
    EGO_CHECK_INITIALIZATION(m_engineSession.isExpired());
    EGO_CHECK_INITIALIZATION(!m_level);
    EGO_CHECK_INITIALIZATION(m_viewport.isExpired());
    EGO_CHECK_INITIALIZATION(m_windows.empty());

    m_engineSession = _initData.m_engineSession;
    const engine::EngineSessionPointer engineSession = m_engineSession.lock();
    EGO_CHECK_INITIALIZATION(engineSession);

    m_level = engineSession->getLevelController().createLevel();
    EGO_CHECK_INITIALIZATION(m_level);
    EGO_CHECK_INITIALIZATION(engineSession->getLevelController().setActiveLevel(m_level->getID()));

    m_cameraEntity = m_level->createNode();
    EGO_CHECK_INITIALIZATION(m_cameraEntity);
    EGO_CHECK_INITIALIZATION(m_level->addOrReplaceComponent<render::CameraComponent>(m_cameraEntity));
    engineSession->setRenderCameraEntity(m_cameraEntity);

    const gui::GuiControllerPointer guiController = engineSession->getGuiControllerPointer();
    EGO_CHECK_INITIALIZATION(guiController);

    const gui::ViewportPointer primaryViewport = guiController->getPrimaryViewport();
    EGO_CHECK_INITIALIZATION(primaryViewport);
    m_wasDockingEnabled = primaryViewport->isDockingEnabled();
    EGO_CHECK_INITIALIZATION(primaryViewport->setDockingEnabled(true));
    m_viewport = primaryViewport;

    const gui::WindowPointer demoWindow = createWindow();
    const gui::WindowPointer sceneWindow =
        createToolWindow("Scene", gui::Rect(440.0f, 15.0f, 280.0f, 180.0f), "Drag this window over the docked area and choose a docking target.");
    const gui::WindowPointer inspectorWindow = createToolWindow(
        "Inspector",
        gui::Rect(440.0f, 210.0f, 280.0f, 220.0f),
        "Drop into the center to create a tab, or onto a side target to split the space.");
    EGO_CHECK_INITIALIZATION(demoWindow && sceneWindow && inspectorWindow);

    const gui::DockingSpaceID defaultSpaceID = primaryViewport->getDefaultDockingSpaceID();
    EGO_CHECK_INITIALIZATION(defaultSpaceID != gui::InvalidDockingSpaceID);

    m_windows = {demoWindow, sceneWindow, inspectorWindow};
    EGO_CHECK_INITIALIZATION(primaryViewport->addWindow(demoWindow, {.m_spaceID = defaultSpaceID}));
    EGO_CHECK_INITIALIZATION(primaryViewport->addWindow(sceneWindow));
    EGO_CHECK_INITIALIZATION(primaryViewport->addWindow(inspectorWindow));

    return true;
}

void ego::demo::GuiDemo::update(float)
{
    const engine::EngineSessionPointer engineSession = m_engineSession.lock();
    if (!engineSession || !m_level)
    {
        return;
    }

    render::Render& render = engineSession->getRender();
    render.drawLine(TriangleTop, TriangleBottomLeft, TriangleColor);
    render.drawLine(TriangleBottomLeft, TriangleBottomRight, TriangleColor);
    render.drawLine(TriangleBottomRight, TriangleTop, TriangleColor);
}

void ego::demo::GuiDemo::release()
{
    const gui::ViewportPointer viewport = m_viewport.lock();
    if (viewport)
    {
        for (const gui::WindowPointer& window : m_windows)
        {
            viewport->removeWindow(window);
        }
        if (!m_wasDockingEnabled)
        {
            viewport->setDockingEnabled(false);
        }
    }

    m_windows.clear();
    m_viewport.reset();
    m_wasDockingEnabled = false;

    const engine::EngineSessionPointer engineSession = m_engineSession.lock();
    if (engineSession && m_level)
    {
        const LevelPointer activeLevel = engineSession->getLevelController().getActiveLevel();
        if (activeLevel && activeLevel->getID() == m_level->getID())
        {
            engineSession->getLevelController().clearActiveLevel();
        }

        engineSession->clearRenderCameraEntity();
    }

    m_level = nullptr;
    m_cameraEntity = ecs::Entity();
    m_engineSession.reset();
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
        resolution->setText(
            "Window resolution: " + std::to_string(static_cast<uint32_t>(_size.m_x)) + " x " + std::to_string(static_cast<uint32_t>(_size.m_y)));
    };

    EGO_CHECK_RETURN_NULL(
        resolution && textInputState && textInputCommitState && checkBoxState && radioGroupState && buttonState && scrollState && textInput && checkBox &&
        radioGroup && button && buttonRow && labeledTextInput && panel && scrollView);

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

ego::gui::WindowPointer ego::demo::GuiDemo::createToolWindow(const std::string& _title, const gui::Rect& _bounds, const std::string& _description) const
{
    const gui::TextPointer description = gui::Text::Create(_description);
    EGO_CHECK_RETURN_NULL(description);

    return gui::Window::Create(
        {
            .m_title = _title,
            .m_bounds = _bounds,
            .m_content = description,
        });
}

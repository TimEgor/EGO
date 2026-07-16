#include "GuiDemo.h"

#include "EgoCore/UtilsMacros.h"

#include "EgoEngine/EngineSession.h"

bool ego::demo::GuiDemo::init(const InitData& _initData)
{
    EGO_CHECK_INITIALIZATION(!_initData.m_engineSession.isExpired());
    EGO_CHECK_INITIALIZATION(m_engineSession.isExpired());

    m_engineSession = _initData.m_engineSession;
    const engine::EngineSessionPointer engineSession = m_engineSession.lock();
    EGO_CHECK_INITIALIZATION(engineSession);

    gui::GuiController& guiController = engineSession->getGuiController();
    const gui::GuiViewportPointer primaryViewport = guiController.getPrimaryViewport();
    EGO_CHECK_INITIALIZATION(primaryViewport);

    m_sceneSettingsWindow = createGuiWindow("Scene Settings", gui::GuiPosition(20.0f, 20.0f), gui::GuiSize(360.0f, 400.0f));
    EGO_CHECK_INITIALIZATION(m_sceneSettingsWindow);
    EGO_CHECK_INITIALIZATION(primaryViewport->addWindow(m_sceneSettingsWindow));

    m_renderSettingsWindow = createGuiWindow("Render Settings", gui::GuiPosition(140.0f, 80.0f), gui::GuiSize(320.0f, 300.0f));
    EGO_CHECK_INITIALIZATION(m_renderSettingsWindow);
    EGO_CHECK_INITIALIZATION(primaryViewport->addWindow(m_renderSettingsWindow));

    return true;
}

void ego::demo::GuiDemo::update(float _deltaTime)
{
    (void)_deltaTime;
}

void ego::demo::GuiDemo::release()
{
    const engine::EngineSessionPointer engineSession = m_engineSession.lock();
    if (engineSession)
    {
        const gui::GuiControllerPointer guiController = engineSession->getGuiControllerPointer();
        if (guiController && guiController->isInitialized())
        {
            const gui::GuiViewportPointer primaryViewport = guiController->getPrimaryViewport();
            if (primaryViewport)
            {
                primaryViewport->removeWindow(m_sceneSettingsWindow);
                primaryViewport->removeWindow(m_renderSettingsWindow);
            }
        }
    }

    m_sceneSettingsWindow = nullptr;
    m_renderSettingsWindow = nullptr;
    m_engineSession.reset();
}

ego::gui::GuiWindowPointer ego::demo::GuiDemo::createGuiWindow(const std::string& _title, const gui::GuiPosition& _position, const gui::GuiSize& _size)
{
    const gui::GuiVerticalBoxPointer panel = gui::GuiVerticalBox::Create();
    EGO_CHECK_RETURN_NULL(panel);

    const gui::GuiTextInputPointer objectName = gui::GuiTextInput::Create();
    EGO_CHECK_RETURN_NULL(objectName);
    objectName->setName("Object Name");
    objectName->setText("Directional Light");
    objectName->setPlaceholder("Enter name");
    EGO_CHECK_RETURN_NULL(panel->addChild(objectName, gui::GuiBoxLayout::Content(gui::GuiMargin(12.0f, 4.0f))));

    const gui::GuiCheckBoxPointer shadowsEnabled = gui::GuiCheckBox::Create();
    EGO_CHECK_RETURN_NULL(shadowsEnabled);
    shadowsEnabled->setText("Enable Shadows");
    shadowsEnabled->setChecked(true);
    EGO_CHECK_RETURN_NULL(panel->addChild(shadowsEnabled, gui::GuiBoxLayout::Content(gui::GuiMargin(12.0f, 4.0f))));

    const gui::GuiRadioGroupPointer renderModeGroup = gui::GuiRadioGroup::Create();
    EGO_CHECK_RETURN_NULL(renderModeGroup);
    renderModeGroup->setTitle("Render Mode");
    renderModeGroup->addOption("Shaded");
    renderModeGroup->addOption("Wireframe");
    renderModeGroup->addOption("Lighting Only");
    EGO_CHECK_RETURN_NULL(panel->addChild(renderModeGroup, gui::GuiBoxLayout::Content(gui::GuiMargin(12.0f, 4.0f))));

    const gui::GuiButtonPointer button = gui::GuiButton::Create();
    EGO_CHECK_RETURN_NULL(button);
    button->setText("Apply");
    button->setOnClicked(
        []()
        {
            return gui::GuiEventResult::Handled;
        });
    EGO_CHECK_RETURN_NULL(panel->addChild(button, gui::GuiBoxLayout::Content(gui::GuiMargin(12.0f))));

    const gui::GuiScrollBoxPointer scrollBox = gui::GuiScrollBox::Create();
    EGO_CHECK_RETURN_NULL(scrollBox);
    scrollBox->setHorizontalScrollBarMode(gui::GuiScrollBarMode::Auto);
    scrollBox->setContent(panel);

    const gui::GuiWindowPointer window = gui::GuiWindow::Create();
    EGO_CHECK_RETURN_NULL(window);
    window->setTitle(_title);
    window->setPosition(_position);
    window->setSize(_size);
    window->setContent(scrollBox);

    return window;
}

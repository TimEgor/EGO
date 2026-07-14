#include "GuiDemo.h"

#include "EgoCore/UtilsMacros.h"

#include "EgoGui/Docking/GuiDockSpace.h"
#include "EgoGui/Widgets/GuiButton.h"
#include "EgoGui/Widgets/GuiCheckBox.h"
#include "EgoGui/Widgets/GuiRadioGroup.h"
#include "EgoGui/Widgets/GuiTextBlock.h"
#include "EgoGui/Widgets/GuiTextInput.h"
#include "EgoGui/Widgets/GuiVerticalBox.h"

#include "EgoEngine/EngineSession.h"
#include "EgoEngine/Graphic/Render/Component/CameraComponent.h"

namespace
{
    constexpr ego::gui::GuiDockTabID DemoTabID = 1;
} // namespace

bool ego::demo::GuiDemo::init(const InitData& _initData)
{
    EGO_CHECK_INITIALIZATION(!_initData.m_engineSession.isExpired());
    EGO_CHECK_INITIALIZATION(m_engineSession.isExpired());

    m_engineSession = _initData.m_engineSession;
    const engine::EngineSessionPointer engineSession = m_engineSession.lock();
    EGO_CHECK_INITIALIZATION(engineSession);

    EGO_CHECK_INITIALIZATION(createGuiTree(engineSession));
    EGO_CHECK_INITIALIZATION(createLevel(engineSession));

    return true;
}

void ego::demo::GuiDemo::update(float _deltaTime)
{
    (void)_deltaTime;

    const engine::EngineSessionPointer engineSession = m_engineSession.lock();
    EGO_CHECK_RETURN(engineSession);

    gui::GuiController& guiController = engineSession->getGuiController();
    if (!guiController.isInitialized() || !guiController.getViewport())
    {
        return;
    }

    guiController.beginFrame();
    guiController.endFrame();
}

void ego::demo::GuiDemo::release()
{
    const engine::EngineSessionPointer engineSession = m_engineSession.lock();
    if (engineSession)
    {
        const gui::GuiControllerPointer guiController = engineSession->getGuiControllerPointer();
        if (guiController && guiController->isInitialized() && guiController->getViewport())
        {
            guiController->getViewport()->setRootWidget(nullptr);
        }

        if (m_level)
        {
            const LevelPointer activeLevel = engineSession->getLevelController().getActiveLevel();
            if (activeLevel && activeLevel->getID() == m_level->getID())
            {
                engineSession->getLevelController().clearActiveLevel();
            }

            engineSession->clearRenderCameraEntity();
        }
    }

    m_level = nullptr;
    m_cameraEntity = ecs::Entity();
    m_engineSession.reset();
}

bool ego::demo::GuiDemo::createGuiTree(const engine::EngineSessionPointer& _engineSession)
{
    const gui::GuiDockSpacePointer dockSpace = gui::GuiDockSpace::Create();
    EGO_CHECK_RETURN_FALSE(dockSpace);

    const gui::GuiVerticalBoxPointer panel = gui::GuiVerticalBox::Create();
    EGO_CHECK_RETURN_FALSE(panel);

    const gui::GuiTextBlockPointer title = gui::GuiTextBlock::Create();
    EGO_CHECK_RETURN_FALSE(title);
    title->setText("Scene Settings");
    panel->addSlot(title).setPadding(gui::GuiMargin(12.0f));

    const gui::GuiTextInputPointer objectName = gui::GuiTextInput::Create();
    EGO_CHECK_RETURN_FALSE(objectName);
    objectName->setName("Object Name");
    objectName->setText("Directional Light");
    objectName->setPlaceholder("Enter name");
    panel->addSlot(objectName).setPadding(gui::GuiMargin(12.0f, 4.0f));

    const gui::GuiCheckBoxPointer dockingEnabled = gui::GuiCheckBox::Create();
    EGO_CHECK_RETURN_FALSE(dockingEnabled);
    dockingEnabled->setText("Enable Shadows");
    dockingEnabled->setChecked(true);
    panel->addSlot(dockingEnabled).setPadding(gui::GuiMargin(12.0f, 4.0f));

    const gui::GuiRadioGroupPointer renderModeGroup = gui::GuiRadioGroup::Create();
    EGO_CHECK_RETURN_FALSE(renderModeGroup);
    renderModeGroup->setTitle("Render Mode");
    renderModeGroup->addOption("Shaded");
    renderModeGroup->addOption("Wireframe");
    renderModeGroup->addOption("Lighting Only");
    panel->addSlot(renderModeGroup).setPadding(gui::GuiMargin(12.0f, 4.0f));

    const gui::GuiButtonPointer button = gui::GuiButton::Create();
    EGO_CHECK_RETURN_FALSE(button);
    button->setText("Apply");
    button->setOnClicked(
        []()
        {
            return gui::GuiReply::Handled();
        });
    panel->addSlot(button).setPadding(gui::GuiMargin(12.0f));

    gui::GuiDockTabDesc tabDesc;
    tabDesc.m_id = DemoTabID;
    tabDesc.m_title = "Inspector";
    tabDesc.m_content = panel;
    EGO_CHECK_RETURN_FALSE(dockSpace->openTab(tabDesc));

    const gui::GuiViewportPointer guiViewport = _engineSession->getGuiController().getViewport();
    EGO_CHECK_RETURN_FALSE(guiViewport);

    guiViewport->setRootWidget(dockSpace);
    return true;
}

bool ego::demo::GuiDemo::createLevel(const engine::EngineSessionPointer& _engineSession)
{
    m_level = _engineSession->getLevelController().createLevel();
    EGO_CHECK_RETURN_FALSE(m_level);
    EGO_CHECK_RETURN_FALSE(_engineSession->getLevelController().setActiveLevel(m_level->getID()));

    m_cameraEntity = m_level->createNode();
    EGO_CHECK_RETURN_FALSE(m_cameraEntity);

    EGO_CHECK_RETURN_FALSE(m_level->addOrReplaceComponent<render::CameraComponent>(m_cameraEntity));
    _engineSession->setRenderCameraEntity(m_cameraEntity);

    return true;
}

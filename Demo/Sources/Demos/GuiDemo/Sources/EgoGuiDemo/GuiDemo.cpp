#include "GuiDemo.h"

#include "EgoCore/Math/Color.h"
#include "EgoCore/UtilsMacros.h"

#include "EgoEngine/EngineSession.h"
#include "EgoEngine/Graphic/SceneRender/Component/CameraComponent.h"
#include "EgoEngine/Graphic/SceneRender/Render.h"

namespace
{
    constexpr ego::NormalizedColorRGB TriangleColor(0.1f, 0.8f, 1.0f);
    constexpr auto TriangleTop = ego::FloatVector3(0.0f, 0.6f, 0.0f);
    constexpr auto TriangleBottomLeft = ego::FloatVector3(-0.6f, -0.5f, 0.0f);
    constexpr auto TriangleBottomRight = ego::FloatVector3(0.6f, -0.5f, 0.0f);
} // namespace

ego::demo::GuiDemo::~GuiDemo()
{
    release();
}

bool ego::demo::GuiDemo::init(const engine::EngineSessionWeakPointer& _engineSession)
{
    EGO_CHECK_INITIALIZATION(!_engineSession.isExpired());
    EGO_CHECK_INITIALIZATION(m_engineSession.isExpired());
    EGO_CHECK_INITIALIZATION(!m_level);

    m_engineSession = _engineSession;
    const engine::EngineSessionPointer engineSession = m_engineSession.lock();
    EGO_CHECK_INITIALIZATION(engineSession);

    const gui::GuiControllerPointer guiController = engineSession->getGuiControllerPointer();
    EGO_CHECK_INITIALIZATION(guiController);

    EGO_CHECK_INITIALIZATION(guiController->registerLayer(m_guiLayer));

    m_level = MakePointer<Level>();
    EGO_CHECK_INITIALIZATION(m_level);
    EGO_CHECK_INITIALIZATION(engineSession->setActiveLevel(m_level));

    m_cameraEntity = m_level->createNode();
    EGO_CHECK_INITIALIZATION(m_cameraEntity);
    EGO_CHECK_INITIALIZATION(m_level->addOrReplaceComponent<render::CameraComponent>(m_cameraEntity));
    engineSession->setRenderCameraEntity(m_cameraEntity);

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
    const engine::EngineSessionPointer engineSession = m_engineSession.lock();
    const gui::GuiControllerPointer guiController = engineSession ? engineSession->getGuiControllerPointer() : nullptr;

    if (guiController)
    {
        guiController->unregisterLayer(m_guiLayer);
    }

    if (engineSession && m_level)
    {
        const LevelPointer activeLevel = engineSession->getActiveLevel();
        if (activeLevel == m_level)
        {
            engineSession->clearActiveLevel();
        }

        engineSession->clearRenderCameraEntity();
    }

    m_level = nullptr;
    m_cameraEntity = ecs::Entity();
    m_engineSession.reset();

    m_guiLayer.reset();
}

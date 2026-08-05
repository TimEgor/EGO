#include "DebugDrawDemo.h"

#include <cmath>

#include "EgoCore/Math/Color.h"
#include "EgoCore/UtilsMacros.h"

#include "EgoEngine/EngineSession.h"
#include "EgoEngine/Graphic/SceneRender/Component/CameraComponent.h"
#include "EgoEngine/Graphic/SceneRender/Render.h"
#include "EgoEngine/Level/Components/TransformComponent.h"

namespace
{
    constexpr float FullRotation = 6.28318530718f;

    constexpr ego::TransformVector CameraPosition(ego::TransformValue(0.0), ego::TransformValue(0.0), ego::TransformValue(-2.0));
    constexpr ego::NormalizedColorRGB Red(1.0f, 0.1f, 0.1f);
    constexpr ego::NormalizedColorRGB Green(0.1f, 1.0f, 0.1f);
    constexpr ego::NormalizedColorRGB Blue(0.1f, 0.3f, 1.0f);
    constexpr ego::NormalizedColorRGB Cyan(0.0f, 0.9f, 1.0f);
    constexpr ego::NormalizedColorRGB Yellow(1.0f, 0.9f, 0.0f);
    constexpr ego::NormalizedColorRGB Magenta(1.0f, 0.0f, 0.9f);
    constexpr ego::NormalizedColorRGB White = ego::NormalizedColorWhite;
} // namespace

ego::demo::DebugDrawDemo::~DebugDrawDemo()
{
    release();
}

bool ego::demo::DebugDrawDemo::init(const engine::EngineSessionWeakPointer& _engineSession)
{
    EGO_CHECK_INITIALIZATION(!_engineSession.isExpired());
    EGO_CHECK_INITIALIZATION(m_engineSession.isExpired());

    m_engineSession = _engineSession;
    const engine::EngineSessionPointer engineSession = m_engineSession.lock();
    EGO_CHECK_INITIALIZATION(engineSession);

    m_level = MakePointer<Level>();
    EGO_CHECK_INITIALIZATION(m_level);
    EGO_CHECK_INITIALIZATION(engineSession->setActiveLevel(m_level));

    m_cameraEntity = m_level->createNode();
    EGO_CHECK_INITIALIZATION(m_cameraEntity);

    EGO_CHECK_INITIALIZATION(m_level->addOrReplaceComponent<render::CameraComponent>(m_cameraEntity));
    TransformComponent* cameraTransformComponent = m_level->tryGetComponent<TransformComponent>(m_cameraEntity);
    EGO_CHECK_INITIALIZATION(cameraTransformComponent);
    cameraTransformComponent->m_globalTransform.setOrigin(CameraPosition);
    engineSession->setRenderCameraEntity(m_cameraEntity);

    return true;
}

void ego::demo::DebugDrawDemo::update(float _deltaTime)
{
    if (!m_level)
    {
        return;
    }

    m_time += _deltaTime;
    while (m_time >= FullRotation)
    {
        m_time -= FullRotation;
    }

    const engine::EngineSessionPointer engineSession = m_engineSession.lock();
    if (!engineSession)
    {
        return;
    }

    drawStaticPrimitives(engineSession);
    drawAnimatedPrimitives(engineSession);
}

void ego::demo::DebugDrawDemo::release()
{
    const engine::EngineSessionPointer engineSession = m_engineSession.lock();
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
    m_time = 0.0f;
    m_engineSession.reset();
}

void ego::demo::DebugDrawDemo::drawStaticPrimitives(const engine::EngineSessionPointer& _engineSession)
{
    render::Render& render = _engineSession->getRender();

    render.drawLine(FloatVector3(-0.9f, 0.0f, 0.0f), FloatVector3(0.9f, 0.0f, 0.0f), Red);
    render.drawLine(FloatVector3(0.0f, -0.9f, 0.0f), FloatVector3(0.0f, 0.9f, 0.0f), Green);

    render.drawLine(FloatVector3(-0.6f, -0.6f, 0.0f), FloatVector3(0.6f, -0.6f, 0.0f), Cyan);
    render.drawLine(FloatVector3(0.6f, -0.6f, 0.0f), FloatVector3(0.6f, 0.6f, 0.0f), Cyan);
    render.drawLine(FloatVector3(0.6f, 0.6f, 0.0f), FloatVector3(-0.6f, 0.6f, 0.0f), Cyan);
    render.drawLine(FloatVector3(-0.6f, 0.6f, 0.0f), FloatVector3(-0.6f, -0.6f, 0.0f), Cyan);

    render::DebugDrawLineData gradientLine;
    gradientLine.m_start.m_position = FloatVector3(-0.6f, -0.6f, 0.0f);
    gradientLine.m_start.m_color = Yellow;
    gradientLine.m_end.m_position = FloatVector3(0.6f, 0.6f, 0.0f);
    gradientLine.m_end.m_color = Magenta;
    render.drawLine(gradientLine);

    render.drawPoint(FloatVector3(-0.6f, -0.6f, 0.0f), White);
    render.drawPoint(FloatVector3(0.6f, -0.6f, 0.0f), White);
    render.drawPoint(FloatVector3(0.6f, 0.6f, 0.0f), White);
    render.drawPoint(FloatVector3(-0.6f, 0.6f, 0.0f), White);
}

void ego::demo::DebugDrawDemo::drawAnimatedPrimitives(const engine::EngineSessionPointer& _engineSession)
{
    render::Render& render = _engineSession->getRender();

    const float pointX = std::cos(m_time) * 0.45f;
    const float pointY = std::sin(m_time) * 0.45f;
    const FloatVector3 pointPosition(pointX, pointY, 0.0f);
    render.drawPoint(pointPosition, Yellow);

    render::DebugDrawLineData radialLine;
    radialLine.m_start.m_position = FloatVector3Zero;
    radialLine.m_start.m_color = Blue;
    radialLine.m_end.m_position = pointPosition;
    radialLine.m_end.m_color = Yellow;
    render.drawLine(radialLine);
}

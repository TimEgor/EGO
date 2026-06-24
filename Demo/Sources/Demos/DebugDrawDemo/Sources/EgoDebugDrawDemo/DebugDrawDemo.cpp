#include "DebugDrawDemo.h"

#include <cmath>

#include "EgoCore/UtilsMacros.h"

#include "EgoEngine/Engine.h"
#include "EgoEngine/Graphic/Render/Component/CameraComponent.h"
#include "EgoEngine/Graphic/Render/Render.h"

namespace
{
    constexpr float FullRotation = 6.28318530718f;

    constexpr auto Red = ego::FloatVector4(1.0f, 0.1f, 0.1f, 1.0f);
    constexpr auto Green = ego::FloatVector4(0.1f, 1.0f, 0.1f, 1.0f);
    constexpr auto Blue = ego::FloatVector4(0.1f, 0.3f, 1.0f, 1.0f);
    constexpr auto Cyan = ego::FloatVector4(0.0f, 0.9f, 1.0f, 1.0f);
    constexpr auto Yellow = ego::FloatVector4(1.0f, 0.9f, 0.0f, 1.0f);
    constexpr auto Magenta = ego::FloatVector4(1.0f, 0.0f, 0.9f, 1.0f);
    constexpr auto White = ego::FloatVector4(1.0f, 1.0f, 1.0f, 1.0f);
} // namespace

bool ego::demo::DebugDrawDemo::init()
{
    engine::Engine& engine = engine::GetEngine();

    m_level = engine.getLevelController().createLevel();
    EGO_CHECK_INITIALIZATION(m_level);
    EGO_CHECK_INITIALIZATION(engine.getLevelController().setActiveLevel(m_level->getID()));

    m_cameraEntity = m_level->createNode();
    EGO_CHECK_INITIALIZATION(m_cameraEntity);

    EGO_CHECK_INITIALIZATION(m_level->addOrReplaceComponent<render::CameraComponent>(m_cameraEntity));
    engine.setRenderCameraEntity(m_cameraEntity);

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

    drawStaticPrimitives();
    drawAnimatedPrimitives();
}

void ego::demo::DebugDrawDemo::release()
{
    const engine::EnginePointer engine = engine::EngineCore::GetInstance().getEngine();
    if (engine && m_level)
    {
        const LevelPointer activeLevel = engine->getLevelController().getActiveLevel();
        if (activeLevel && activeLevel->getID() == m_level->getID())
        {
            engine->getLevelController().clearActiveLevel();
        }

        engine->clearRenderCameraEntity();
    }

    m_level = nullptr;
    m_cameraEntity = ecs::Entity();
    m_time = 0.0f;
}

void ego::demo::DebugDrawDemo::drawStaticPrimitives()
{
    render::Render& render = engine::GetEngine().getRender();

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

void ego::demo::DebugDrawDemo::drawAnimatedPrimitives()
{
    render::Render& render = engine::GetEngine().getRender();

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

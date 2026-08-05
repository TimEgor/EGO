#include "Render.h"

#include "EgoEngine/Graphic/SceneRender/Component/CameraComponent.h"
#include "EgoEngine/Level/Components/TransformComponent.h"
#include "EgoEngine/Level/Level.h"

bool ego::render::Render::prepare(const RenderPrepareContext& _context)
{
    if (!prepareCamera(_context))
    {
        clearResources();

        return false;
    }

    return onPrepare(_context);
}

void ego::render::Render::drawPoint(const FloatVector3& _position, const NormalizedColorRGB& _color)
{
    DebugDrawPointData point;
    point.m_position = _position;
    point.m_color = _color;
    drawPoint(point);
}

void ego::render::Render::drawLine(const FloatVector3& _startPosition, const FloatVector3& _endPosition, const NormalizedColorRGB& _color)
{
    drawLine(_startPosition, _color, _endPosition, _color);
}

void ego::render::Render::drawLine(
    const FloatVector3& _startPosition,
    const NormalizedColorRGB& _startColor,
    const FloatVector3& _endPosition,
    const NormalizedColorRGB& _endColor)
{
    DebugDrawLineData line;
    line.m_start.m_position = _startPosition;
    line.m_start.m_color = _startColor;
    line.m_end.m_position = _endPosition;
    line.m_end.m_color = _endColor;

    drawLine(line);
}

const ego::render::RenderCamera& ego::render::Render::getCamera() const
{
    return m_camera;
}

bool ego::render::Render::prepareCamera(const RenderPrepareContext& _context)
{
    const CameraComponent* cameraComponent = _context.m_level.tryGetComponent<CameraComponent>(_context.m_cameraEntity);
    const TransformComponent* transformComponent = _context.m_level.tryGetComponent<TransformComponent>(_context.m_cameraEntity);
    if (!cameraComponent || !transformComponent || _context.m_targetSize.m_x == 0 || _context.m_targetSize.m_y == 0)
    {
        m_camera.reset();

        return false;
    }

    const float aspectRatio = static_cast<float>(_context.m_targetSize.m_x) / static_cast<float>(_context.m_targetSize.m_y);

    return m_camera.update(
        transformComponent->m_globalTransform,
        cameraComponent->m_verticalFieldOfView,
        aspectRatio,
        cameraComponent->m_nearPlane,
        cameraComponent->m_farPlane);
}

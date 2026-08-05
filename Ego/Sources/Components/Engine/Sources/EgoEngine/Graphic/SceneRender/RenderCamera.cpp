#include "RenderCamera.h"

#include <cmath>

#include "EgoCore/Math/ComputeMath.h"
#include "EgoCore/Math/ComputeMatrix44.h"

bool ego::render::RenderCamera::update(const Transform& _transform, float _verticalFieldOfView, float _aspectRatio, float _nearPlane, float _farPlane)
{
    if (!std::isfinite(_verticalFieldOfView) || !std::isfinite(_aspectRatio) || !std::isfinite(_nearPlane) || !std::isfinite(_farPlane) ||
        _verticalFieldOfView <= 0.0f || _verticalFieldOfView >= 180.0f || _aspectRatio <= 0.0f || _nearPlane <= 0.0f || _farPlane <= _nearPlane)
    {
        reset();

        return false;
    }

    const FloatMatrix4x4 projectionMatrix = CreatePerspectiveProjectionMatrix(_verticalFieldOfView, _aspectRatio, _nearPlane, _farPlane);
    const ComputeMatrix4x4 projection(projectionMatrix);
    const ComputeMatrix4x4 view = InvertComputeMatrix4x4(ComputeMatrix4x4(_transform.m_matrix));
    const ComputeMatrix4x4 viewProjection = projection * view;
    const ComputeMatrix4x4 inverseViewProjection = InvertComputeMatrix4x4(viewProjection);

    m_transform = _transform;
    m_projection = projectionMatrix;
    m_view = view.getMatrix<float>();
    m_viewProjection = viewProjection.getMatrix<float>();
    m_inverseViewProjection = inverseViewProjection.getMatrix<float>();

    return true;
}

void ego::render::RenderCamera::reset()
{
    m_transform = Transform();
    m_projection = FloatMatrix4x4Identity;
    m_view = FloatMatrix4x4Identity;
    m_viewProjection = FloatMatrix4x4Identity;
    m_inverseViewProjection = FloatMatrix4x4Identity;
}

const ego::Transform& ego::render::RenderCamera::getTransform() const
{
    return m_transform;
}

const ego::FloatMatrix4x4& ego::render::RenderCamera::getProjectionMatrix() const
{
    return m_projection;
}

const ego::FloatMatrix4x4& ego::render::RenderCamera::getViewMatrix() const
{
    return m_view;
}

const ego::FloatMatrix4x4& ego::render::RenderCamera::getViewProjectionMatrix() const
{
    return m_viewProjection;
}

const ego::FloatMatrix4x4& ego::render::RenderCamera::getInverseViewProjectionMatrix() const
{
    return m_inverseViewProjection;
}

ego::FloatMatrix4x4 ego::render::RenderCamera::CreatePerspectiveProjectionMatrix(
    float _verticalFieldOfView,
    float _aspectRatio,
    float _nearPlane,
    float _farPlane)
{
    const ComputeValueType verticalFieldOfView = math::ConvertDegToRad(static_cast<ComputeValueType>(_verticalFieldOfView));
    const ComputeValueType aspectRatio = static_cast<ComputeValueType>(_aspectRatio);
    const ComputeValueType nearPlane = static_cast<ComputeValueType>(_nearPlane);
    const ComputeValueType farPlane = static_cast<ComputeValueType>(_farPlane);

    const ComputeValueType verticalScale = ComputeValueType(1.0) / std::tan(verticalFieldOfView * ComputeValueType(0.5));
    const ComputeValueType horizontalScale = verticalScale / aspectRatio;
    const ComputeValueType depthScale = farPlane / (farPlane - nearPlane);
    const ComputeValueType depthOffset = -nearPlane * depthScale;

    ComputeMatrix4x4 projection = ComputeMatrix4x4ZeroBase<ComputeValueType>();
    projection.setElement(0, 0, horizontalScale);
    projection.setElement(1, 1, verticalScale);
    projection.setElement(2, 2, depthScale);
    projection.setElement(2, 3, depthOffset);
    projection.setElement(3, 2, ComputeValueType(1.0));

    return projection.getMatrix<float>();
}

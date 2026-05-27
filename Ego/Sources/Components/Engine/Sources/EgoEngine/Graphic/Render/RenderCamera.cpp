#include "RenderCamera.h"

const ego::Transform& ego::RenderCamera::getTransform() const
{
    return m_transform;
}

void ego::RenderCamera::setTransform(const Transform& _transform)
{
    m_transform = _transform;
}

const ego::ComputeMatrix4x4& ego::RenderCamera::getProjectionMatrix() const
{
    return m_projection;
}

void ego::RenderCamera::setProjectionMatrix(const ComputeMatrix4x4& _projection)
{
    m_projection = _projection;
}

ego::ComputeMatrix4x4 ego::RenderCamera::getViewMatrix() const
{
    return InvertComputeMatrix4x4(m_transform.m_matrix);
}

ego::ComputeMatrix4x4 ego::RenderCamera::getViewProjectionMatrix() const
{
    return m_projection * getViewMatrix();
}

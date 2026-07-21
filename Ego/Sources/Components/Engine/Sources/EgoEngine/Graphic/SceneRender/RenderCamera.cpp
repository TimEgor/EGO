#include "RenderCamera.h"

const ego::Transform& ego::render::RenderCamera::getTransform() const
{
    return m_transform;
}

void ego::render::RenderCamera::setTransform(const Transform& _transform)
{
    m_transform = _transform;
}

const ego::ComputeMatrix4x4& ego::render::RenderCamera::getProjectionMatrix() const
{
    return m_projection;
}

void ego::render::RenderCamera::setProjectionMatrix(const ComputeMatrix4x4& _projection)
{
    m_projection = _projection;
}

ego::ComputeMatrix4x4 ego::render::RenderCamera::getViewMatrix() const
{
    return InvertComputeMatrix4x4(m_transform.m_matrix);
}

ego::ComputeMatrix4x4 ego::render::RenderCamera::getViewProjectionMatrix() const
{
    return m_projection * getViewMatrix();
}

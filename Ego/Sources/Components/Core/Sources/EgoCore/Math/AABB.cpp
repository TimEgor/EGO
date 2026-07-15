#include "AABB.h"

#include "ComputeVector3.h"

ego::AABB3& ego::AABB3::merge(const AABB3& _volume)
{
    m_minPosition = MinComputeVector3(m_minPosition, _volume.m_minPosition);
    m_maxPosition = MaxComputeVector3(m_maxPosition, _volume.m_maxPosition);

    return *this;
}

ego::Transform ego::AABB3::getTransform() const
{
    const ComputeVector3 delta = m_maxPosition - m_minPosition;
    const ComputeVector3 size = delta * ComputeValue(0.5);
    const ComputeVector3 center = m_minPosition + size;

    Transform transform;
    transform.setOrigin(center);
    transform.m_matrix.getColumn(0).setX(size.getX());
    transform.m_matrix.getColumn(1).setY(size.getY());
    transform.m_matrix.getColumn(2).setZ(size.getZ());

    return transform;
}

ego::ComputeValue ego::AABB3::getVolume() const
{
    const ComputeVector3 delta = m_maxPosition - m_minPosition;

    return delta.getX() * delta.getY() * delta.getZ();
}

bool ego::AABB3::isContained(const AABB3& _volume) const
{
    return m_minPosition.isLessOrEqual(_volume.m_minPosition) && m_maxPosition.isGreaterOrEqual(_volume.m_maxPosition);
}

bool ego::AABB3::isOverlapped(const AABB3& _volume) const
{
    return !m_maxPosition.isAnyLess(_volume.m_minPosition) && !m_minPosition.isAnyGreater(_volume.m_maxPosition);
}

ego::AABB3 ego::AABBFromMerging(const AABB3& _volume1, const AABB3& _volume2)
{
    AABB3 result(_volume1);
    result.merge(_volume2);

    return result;
}

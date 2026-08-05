#include "AABB.h"

#include "ComputeVector3.h"

ego::AABB3& ego::AABB3::merge(const AABB3& _volume)
{
    const ComputeVector3 minPosition = MinComputeVector3(ComputeVector3(m_minPosition), ComputeVector3(_volume.m_minPosition));
    const ComputeVector3 maxPosition = MaxComputeVector3(ComputeVector3(m_maxPosition), ComputeVector3(_volume.m_maxPosition));

    m_minPosition = minPosition.getVector<AABBValue>();
    m_maxPosition = maxPosition.getVector<AABBValue>();

    return *this;
}

ego::Transform ego::AABB3::getTransform() const
{
    const ComputeVector3 delta = ComputeVector3(m_maxPosition) - ComputeVector3(m_minPosition);
    const ComputeVector3 size = delta * ComputeValue(0.5);
    const ComputeVector3 center = ComputeVector3(m_minPosition) + size;

    Transform transform;
    transform.setOrigin(center.getVector<TransformValue>());
    transform.setAxisX(TransformVector(size.getX(), TransformValue(0.0), TransformValue(0.0)));
    transform.setAxisY(TransformVector(TransformValue(0.0), size.getY(), TransformValue(0.0)));
    transform.setAxisZ(TransformVector(TransformValue(0.0), TransformValue(0.0), size.getZ()));

    return transform;
}

ego::AABBValue ego::AABB3::getVolume() const
{
    const ComputeVector3 delta = ComputeVector3(m_maxPosition) - ComputeVector3(m_minPosition);

    return delta.getX() * delta.getY() * delta.getZ();
}

bool ego::AABB3::contains(const AABB3& _volume) const
{
    const ComputeVector3 minPosition(m_minPosition);
    const ComputeVector3 maxPosition(m_maxPosition);

    return minPosition.isLessOrEqual(ComputeVector3(_volume.m_minPosition)) && maxPosition.isGreaterOrEqual(ComputeVector3(_volume.m_maxPosition));
}

bool ego::AABB3::overlaps(const AABB3& _volume) const
{
    const ComputeVector3 minPosition(m_minPosition);
    const ComputeVector3 maxPosition(m_maxPosition);

    return !maxPosition.isAnyLess(ComputeVector3(_volume.m_minPosition)) && !minPosition.isAnyGreater(ComputeVector3(_volume.m_maxPosition));
}

bool ego::AABB3::isContained(const AABB3& _volume) const
{
    return contains(_volume);
}

bool ego::AABB3::isOverlapped(const AABB3& _volume) const
{
    return overlaps(_volume);
}

ego::AABB3 ego::MergeAABBs(const AABB3& _volume1, const AABB3& _volume2)
{
    AABB3 result(_volume1);
    result.merge(_volume2);

    return result;
}

ego::AABB3 ego::AABBFromMerging(const AABB3& _volume1, const AABB3& _volume2)
{
    return MergeAABBs(_volume1, _volume2);
}

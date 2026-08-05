#pragma once

#include "Transform.h"
#include "Vector.h"

namespace ego
{
    struct AABB3 final
    {
        FloatVector3 m_minPosition = FloatVector3Zero;
        FloatVector3 m_maxPosition = FloatVector3Zero;

        constexpr AABB3() = default;

        constexpr AABB3(const FloatVector3& _min, const FloatVector3& _max)
            : m_minPosition(_min),
              m_maxPosition(_max)
        {
        }

        AABB3& merge(const AABB3& _volume);

        Transform getTransform() const;

        ComputeValue getVolume() const;

        bool isContained(const AABB3& _volume) const;
        bool isOverlapped(const AABB3& _volume) const;
    };

    AABB3 AABBFromMerging(const AABB3& _volume1, const AABB3& _volume2);
} // namespace ego

#pragma once

#include "Transform.h"
#include "Vector.h"

namespace ego
{
    using AABBValue = TransformValue;
    using AABBVector = TransformVector;

    struct AABB3 final
    {
        AABBVector m_minPosition = {};
        AABBVector m_maxPosition = {};

        constexpr AABB3() = default;

        constexpr AABB3(const AABBVector& _min, const AABBVector& _max)
            : m_minPosition(_min),
              m_maxPosition(_max)
        {
        }

        AABB3& merge(const AABB3& _volume);

        Transform getTransform() const;

        AABBValue getVolume() const;

        bool contains(const AABB3& _volume) const;
        bool overlaps(const AABB3& _volume) const;
        bool isContained(const AABB3& _volume) const;
        bool isOverlapped(const AABB3& _volume) const;
    };

    AABB3 MergeAABBs(const AABB3& _volume1, const AABB3& _volume2);
    AABB3 AABBFromMerging(const AABB3& _volume1, const AABB3& _volume2);
} // namespace ego

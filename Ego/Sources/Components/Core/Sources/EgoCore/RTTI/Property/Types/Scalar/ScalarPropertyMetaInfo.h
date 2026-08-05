#pragma once

#include <type_traits>

#include "EgoCore/RTTI/Property/PropertyMetaInfo.h"

namespace ego::rtti
{
    template <typename T>
        requires std::is_arithmetic_v<T>
    class ScalarPropertyMetaInfo final : public PropertyMetaInfo
    {
    public:
        ScalarPropertyMetaInfo(const char* _name, size_t _offset);

        EGO_RTTI_VIRTUAL(ScalarPropertyMetaInfo, PropertyMetaInfo);
    };

    using BoolPropertyMetaInfo = ScalarPropertyMetaInfo<bool>;
    using FloatPropertyMetaInfo = ScalarPropertyMetaInfo<float>;
    using DoublePropertyMetaInfo = ScalarPropertyMetaInfo<double>;
} // namespace ego::rtti

#include "EgoCore/RTTI/Property/Types/Scalar/ScalarPropertyMetaInfo.hpp"

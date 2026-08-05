#pragma once

#include <cstdint>

namespace ego::rtti
{
    using TypeMetaInfoID = uint32_t;
    inline constexpr TypeMetaInfoID InvalidTypeMetaInfoID = 0;

    template <typename T>
    constexpr TypeMetaInfoID GetTypeMetaInfoID();

    constexpr TypeMetaInfoID GetTypeMetaInfoID(const char* _typeName);
} // namespace ego::rtti

#include "EgoCore/RTTI/Type/TypeMetaInfoID.hpp"

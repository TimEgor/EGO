#pragma once

#include <cstddef>
#include <utility>

#include "EgoCore/RTTI/Property/PropertyMetaInfo.h"

namespace ego::rtti
{
    template <typename Value>
    PropertyMetaInfoPointer MakePropertyMetaInfo(const char* _name, size_t _offset);

    template <typename... Properties>
    PropertyMetaInfoCollection MakePropertyMetaInfoCollection(Properties&&... _properties);
} // namespace ego::rtti

#include "EgoCore/RTTI/Property/PropertyMetaInfoFactory.hpp"

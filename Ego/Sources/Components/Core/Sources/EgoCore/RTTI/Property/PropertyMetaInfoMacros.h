#pragma once

#include <cstddef>
#include <type_traits>

#include "EgoCore/RTTI/Property/PropertyMetaInfoFactory.h"

#define EGO_RTTI_PROPERTIES(...)                                                                                                                               \
    template <typename T = EgoRttiSelfType>                                                                                                                    \
        requires std::is_same_v<T, EgoRttiSelfType>                                                                                                            \
    static ego::rtti::PropertyMetaInfoCollection RegistryPropertyMetaInfoCollection()                                                                          \
    {                                                                                                                                                          \
        return ego::rtti::MakePropertyMetaInfoCollection(__VA_ARGS__);                                                                                         \
    }

#define EGO_RTTI_PROPERTY(_PROPERTY) ego::rtti::MakePropertyMetaInfo<decltype(EgoRttiSelfType::_PROPERTY)>(#_PROPERTY, offsetof(EgoRttiSelfType, _PROPERTY))

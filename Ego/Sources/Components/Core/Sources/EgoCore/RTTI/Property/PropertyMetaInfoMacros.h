#pragma once

#include <cstddef>
#include <type_traits>

#include "EgoCore/RTTI/Property/PropertyMetaInfoFactory.h"

#define EGO_RTTI_PROPERTIES(...)                                                                                                                               \
    template <typename T = EgoRttiSelfType>                                                                                                                    \
    requires std::is_same_v<T, EgoRttiSelfType>                                                                                                                \
    static ego::rtti::PropertyMetaInfoCollection RegistryPropertyMetaInfoCollection()                                                                          \
    {                                                                                                                                                          \
        ego::rtti::PropertyMetaInfoCollection propertyMetaInfos;                                                                                               \
        __VA_OPT__((__VA_ARGS__);)                                                                                                                             \
        return propertyMetaInfos;                                                                                                                              \
    }

#define EGO_RTTI_PROPERTY(_PROPERTY)                                                                                                                           \
    ego::rtti::AddPropertyMetaInfo<decltype(EgoRttiSelfType::_PROPERTY)>(propertyMetaInfos, #_PROPERTY, offsetof(EgoRttiSelfType, _PROPERTY))

#define EGO_RTTI_PROPERTY_OPTIONS(_PROPERTY, ...)                                                                                                              \
    ego::rtti::AddPropertyMetaInfo<decltype(EgoRttiSelfType::_PROPERTY)>(propertyMetaInfos, #_PROPERTY, offsetof(EgoRttiSelfType, _PROPERTY), __VA_ARGS__)

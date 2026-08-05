#pragma once

#include "EgoCore/RTTI/Type/TypeMetaInfo.h"

#define EGO_RTTI(_TYPE, ...)                                                                                                                                   \
    using EgoRttiSelfType = _TYPE;                                                                                                                             \
                                                                                                                                                               \
    static const char* GetMetaInfoTypeName()                                                                                                                   \
    {                                                                                                                                                          \
        return #_TYPE;                                                                                                                                         \
    }                                                                                                                                                          \
                                                                                                                                                               \
    static constexpr ego::rtti::TypeMetaInfoID GetMetaInfoID()                                                                                                 \
    {                                                                                                                                                          \
        return ego::rtti::GetTypeMetaInfoID(__FUNCSIG__);                                                                                                      \
    }                                                                                                                                                          \
                                                                                                                                                               \
    static const ego::rtti::TypeMetaInfo& GetMetaInfo()                                                                                                        \
    {                                                                                                                                                          \
        static const ego::rtti::TypeMetaInfo TypeMetaInfo = ego::rtti::RegistryTypeMetaInfo<_TYPE __VA_OPT__(, ) __VA_ARGS__>();                               \
        return TypeMetaInfo;                                                                                                                                   \
    }

#define EGO_RTTI_INSTANCE(_PRE_MODIFICATOR, _POST_MODIFICATOR)                                                                                                 \
    _PRE_MODIFICATOR const char* getObjectTypeInfoName() const _POST_MODIFICATOR                                                                               \
    {                                                                                                                                                          \
        return GetMetaInfoTypeName();                                                                                                                          \
    }                                                                                                                                                          \
                                                                                                                                                               \
    _PRE_MODIFICATOR ego::rtti::TypeMetaInfoID getObjectTypeMetaInfoID() const _POST_MODIFICATOR                                                               \
    {                                                                                                                                                          \
        return GetMetaInfoID();                                                                                                                                \
    }                                                                                                                                                          \
                                                                                                                                                               \
    _PRE_MODIFICATOR const ego::rtti::TypeMetaInfo& getObjectTypeMetaInfo() const _POST_MODIFICATOR                                                            \
    {                                                                                                                                                          \
        return GetMetaInfo();                                                                                                                                  \
    }

#define EGO_RTTI_VIRTUAL(_TYPE, ...)                                                                                                                           \
    EGO_RTTI(_TYPE, __VA_ARGS__)                                                                                                                               \
    EGO_RTTI_INSTANCE(virtual, override)

#define EGO_RTTI_VIRTUAL_BASE(_TYPE)                                                                                                                           \
    EGO_RTTI(_TYPE)                                                                                                                                            \
    EGO_RTTI_INSTANCE(virtual, )

#define EGO_RTTI_TYPE_NAME(_TYPE) (ego::rtti::GetTypeMetaInfoName<_TYPE>())
#define EGO_RTTI_TYPE_ID(_TYPE) (ego::rtti::GetTypeMetaInfoID<_TYPE>())
#define EGO_RTTI_TYPE_META_INFO(_TYPE) (ego::rtti::GetTypeMetaInfo<_TYPE>())

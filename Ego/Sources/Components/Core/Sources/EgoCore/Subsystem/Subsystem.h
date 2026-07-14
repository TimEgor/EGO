#pragma once

#include "EgoCore/Reference/Pointer.h"
#include "EgoCore/RTTI/RTTI.h"

namespace ego::subsystem
{
    using SubsystemType = rtti::TypeMetaInfoID;
    inline constexpr SubsystemType InvalidSubsystemType = rtti::InvalidTypeMetaInfoID;

    class Subsystem
    {
    public:
        Subsystem() = default;
        virtual ~Subsystem() = default;

        virtual void release() = 0;
        virtual SubsystemType getType() const = 0;

        EGO_RTTI_VIRTUAL_BASE(Subsystem);
    };

    EGO_POINTER(Subsystem);
} // namespace ego::subsystem

#define EGO_SUBSYSTEM(_SUBSYSTEM, ...)                                                                                                                                             \
    EGO_RTTI_VIRTUAL(_SUBSYSTEM, __VA_ARGS__)                                                                                                                                      \
                                                                                                                                                                                   \
    static const char* GetSubsystemTypeName()                                                                                                                                      \
    {                                                                                                                                                                              \
        return GetMetaInfoTypeName();                                                                                                                                              \
    }                                                                                                                                                                              \
                                                                                                                                                                                   \
    static ego::subsystem::SubsystemType GetSubsystemType()                                                                                                                        \
    {                                                                                                                                                                              \
        return GetMetaInfoID();                                                                                                                                                    \
    }                                                                                                                                                                              \
                                                                                                                                                                                   \
    virtual ego::subsystem::SubsystemType getType() const override                                                                                                                 \
    {                                                                                                                                                                              \
        return GetSubsystemType();                                                                                                                                                 \
    }

#define EGO_SUBSYSTEM_TYPE(_SUBSYSTEM) (_SUBSYSTEM::GetSubsystemType())
#define EGO_SUBSYSTEM_TYPE_NAME(_SUBSYSTEM) (_SUBSYSTEM::GetSubsystemTypeName())

#pragma once

#include "EgoCore/RTTI/Property/PropertyMetaInfo.h"
#include "EgoCore/RTTI/Type/TypeMetaInfoMacros.h"

#include <entt/core/type_info.hpp>

namespace ego::ecs
{
    using ComponentTypeID = entt::id_type;

    struct Component
    {
        virtual ~Component() = default;

        EGO_RTTI_VIRTUAL_BASE(Component);
    };

    inline constexpr ComponentTypeID InvalidComponentTypeID = entt::type_hash<Component>::value();

    template <typename TComponent>
    constexpr ComponentTypeID GetComponentTypeID()
    {
        constexpr ComponentTypeID componentTypeID = entt::type_hash<TComponent>::value();
        static_assert(componentTypeID != InvalidComponentTypeID, "ECS component type ID collides with the invalid component type ID.");

        return componentTypeID;
    }
} // namespace ego::ecs

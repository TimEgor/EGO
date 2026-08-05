#pragma once

#include <string>

#include "EgoCore/RTTI/RTTI.h"

#include "EgoECS/Component.h"

namespace ego
{
    struct NameComponent final : public ecs::Component
    {
        NameComponent() = default;
        explicit NameComponent(std::string _name);

        std::string m_name;

        EGO_RTTI_VIRTUAL(NameComponent, ecs::Component);
        EGO_RTTI_PROPERTIES(EGO_RTTI_PROPERTY(m_name));
    };
} // namespace ego

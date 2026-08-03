#pragma once

#include <string>

#include "EgoECS/Component.h"

namespace ego
{
    struct NameComponent final : public ecs::Component
    {
        NameComponent() = default;
        explicit NameComponent(std::string _name);

        std::string m_name;
    };
} // namespace ego

#pragma once

#include <cstdint>
#include <string>

#include "EgoECS/Component.h"

namespace ego
{
    using LevelID = uint32_t;

    inline constexpr LevelID InvalidLevelID = 0;
    inline constexpr LevelID FirstLevelID = 1;

    struct LevelComponent final : public ecs::Component
    {
        LevelComponent() = default;
        explicit LevelComponent(LevelID _levelID);

        LevelID m_levelID = InvalidLevelID;
    };

    struct NameComponent final : public ecs::Component
    {
        NameComponent() = default;
        explicit NameComponent(std::string _name);

        std::string m_name;
    };
} // namespace ego

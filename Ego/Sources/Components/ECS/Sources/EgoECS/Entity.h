#pragma once

#include <cstdint>
#include <limits>

namespace ego::ecs
{
    using WorldID = uint64_t;
    using EntityID = uint32_t;

    inline constexpr WorldID InvalidWorldID = 0;
    inline constexpr WorldID FirstWorldID = 1;
    inline constexpr EntityID InvalidEntityID = std::numeric_limits<EntityID>::max();

    class Entity final
    {
    public:
        constexpr Entity() = default;
        constexpr Entity(WorldID _worldID, EntityID _id)
            : m_worldID(_worldID),
              m_id(_id)
        {
        }

        constexpr WorldID getWorldID() const
        {
            return m_worldID;
        }

        constexpr EntityID getID() const
        {
            return m_id;
        }
        constexpr bool isValid() const
        {
            return m_worldID != InvalidWorldID && m_id != InvalidEntityID;
        }

        explicit constexpr operator bool() const
        {
            return isValid();
        }

        constexpr bool operator==(Entity _entity) const
        {
            return m_worldID == _entity.m_worldID && m_id == _entity.m_id;
        }
        constexpr bool operator!=(Entity _entity) const
        {
            return !(*this == _entity);
        }
        constexpr bool operator<(Entity _entity) const
        {
            return m_worldID < _entity.m_worldID ||
                   (m_worldID == _entity.m_worldID && m_id < _entity.m_id);
        }

    private:
        WorldID m_worldID = InvalidWorldID;
        EntityID m_id = InvalidEntityID;
    };
} // namespace ego::ecs

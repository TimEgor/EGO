#pragma once

#include <cstdint>
#include <limits>

namespace ego::ecs
{
    using EntityID = uint32_t;

    inline constexpr EntityID InvalidEntityID = std::numeric_limits<EntityID>::max();

    class Entity final
    {
    public:
        constexpr Entity() = default;
        explicit constexpr Entity(EntityID _id) : m_id(_id) {}

        constexpr EntityID getID() const { return m_id; }
        constexpr bool isValid() const { return m_id != InvalidEntityID; }

        explicit constexpr operator bool() const { return isValid(); }

        constexpr bool operator==(Entity _entity) const { return m_id == _entity.m_id; }
        constexpr bool operator!=(Entity _entity) const { return m_id != _entity.m_id; }
        constexpr bool operator<(Entity _entity) const { return m_id < _entity.m_id; }

    private:
        EntityID m_id = InvalidEntityID;
    };
}

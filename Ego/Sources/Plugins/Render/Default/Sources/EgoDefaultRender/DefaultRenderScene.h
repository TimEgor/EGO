#pragma once

#include <cstdint>
#include <vector>

#include "EgoCore/Math/Transform.h"

#include "EgoEngine/Graphic/Render/RenderObject.h"

namespace ego
{
    class Level;
}

namespace ego::render
{
    struct DefaultRenderItem final
    {
        RenderMesh m_mesh = nullptr;
        RenderMaterial m_material = nullptr;
        Transform m_globalTransform;
        uint32_t m_objectIndex = 0;
    };

    class DefaultRenderScene final
    {
    public:
        using ItemCollection = std::vector<DefaultRenderItem>;

        DefaultRenderScene() = default;

        void collect(Level& _level);
        void clear();

        ItemCollection& getItems();
        const ItemCollection& getItems() const;
        bool isEmpty() const;

    private:
        ItemCollection m_items;
    };
} // namespace ego::render

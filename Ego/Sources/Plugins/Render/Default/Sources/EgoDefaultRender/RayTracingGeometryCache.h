#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "EgoEngine/Graphic/SceneRender/Mesh.h"

namespace ego
{
    class GraphicDevice;
}

namespace ego::render
{
    class RayTracingGeometryCache final
    {
    public:
        RayTracingGeometryCache() = default;

        void beginFrame();
        RenderGeometryAccelerationStructure requestGeometry(const MeshHandler& _mesh, ego::GraphicDevice& _graphicDevice);
        void removeUnused();
        void clear();

    private:
        struct Entry final
        {
            MeshHandler m_mesh = nullptr;
            RenderGeometryAccelerationStructure m_geometry = nullptr;
            uint64_t m_lastUsedFrame = 0;
        };

        static bool FillGeometryDesc(const Mesh& _mesh, gpu::MeshAccelerationStructureBuildDesc& _desc);
        size_t findEntryIndex(const MeshHandler& _mesh) const;

        std::vector<Entry> m_entries;
        uint64_t m_frameIndex = 0;
    };
} // namespace ego::render

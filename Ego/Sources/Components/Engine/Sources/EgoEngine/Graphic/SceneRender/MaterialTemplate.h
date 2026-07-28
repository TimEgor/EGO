#pragma once

#include <cstddef>
#include <cstdint>
#include <string_view>
#include <unordered_map>

#include "EgoEngine/Graphic/SceneRender/MaterialRenderPassInfo.h"

namespace ego::render
{
    using MaterialRenderPassId = uint32_t;

    inline constexpr MaterialRenderPassId InvalidMaterialRenderPassId = 0;

    MaterialRenderPassId MakeMaterialRenderPassId(const char* _name);
    MaterialRenderPassId MakeMaterialRenderPassId(const char* _name, std::size_t _length);
    MaterialRenderPassId MakeMaterialRenderPassId(std::string_view _name);

    class MaterialTemplate final : public MTCountable
    {
    public:
        using RenderPassInfoCollection = std::unordered_map<MaterialRenderPassId, MaterialRenderPassInfoPointer>;

        MaterialTemplate() = default;

        bool setRenderPassInfo(MaterialRenderPassId _id, const MaterialRenderPassInfoPointer& _info);
        bool setRenderPassInfo(std::string_view _name, const MaterialRenderPassInfoPointer& _info);

        void removeRenderPassInfo(MaterialRenderPassId _id);
        void removeRenderPassInfo(std::string_view _name);
        void clearRenderPassInfos();

        bool hasRenderPassInfo(MaterialRenderPassId _id) const;
        bool hasRenderPassInfo(std::string_view _name) const;

        MaterialRenderPassInfoPointer getRenderPassInfo(MaterialRenderPassId _id) const;
        MaterialRenderPassInfoPointer getRenderPassInfo(std::string_view _name) const;

        const RenderPassInfoCollection& getRenderPassInfos() const;

    private:
        RenderPassInfoCollection m_renderPassInfos;
    };

    EGO_INTRUSIVE_POINTER(MaterialTemplate);
    EGO_NAMED_HANDLER(MaterialTemplatePointer, MaterialTemplate);
} // namespace ego::render

#pragma once

#include <cstdint>
#include <vector>

#include "GraphicResource.h"
#include "ResourceView.h"
#include "Sampler.h"
#include "Shader.h"

namespace ego::gpu
{
    enum class BindingResourceType
    {
        ConstantBuffer,
        Texture,
        StorageTexture,
        StructuredBuffer,
        StorageBuffer,
        Sampler
    };

    struct BindingElementDesc final
    {
        uint32_t m_set = 0;
        uint32_t m_binding = 0;
        BindingResourceType m_type = BindingResourceType::ConstantBuffer;
        ShaderStageFlag m_stageFlag = ShaderStageFlagNone;
        uint32_t m_count = 1;
    };

    struct PushConstantRangeDesc final
    {
        uint32_t m_offset = 0;
        uint32_t m_size = 0;
        ShaderStageFlag m_stageFlag = ShaderStageFlagNone;
    };

    struct BindingLayoutDesc final
    {
        std::vector<BindingElementDesc> m_bindings;
        std::vector<PushConstantRangeDesc> m_pushConstants;
    };

    class BindingLayout : public GraphicResource
    {
    public:
        BindingLayout(const BindingLayoutDesc& _desc);

        const BindingLayoutDesc& getDesc() const;

        EGO_GRAPHIC_RESOURCE(BindingLayout, GraphicResource);

    private:
        BindingLayoutDesc m_desc;
    };

    EGO_POINTER(BindingLayout);

    struct BindingSetItemDesc final
    {
        uint32_t m_binding = 0;
        std::vector<ResourceViewPointer> m_resourceViews;
        std::vector<SamplerPointer> m_samplers;
        uint64_t m_offset = 0;
        uint64_t m_size = 0;
    };

    struct BindingSetDesc final
    {
        BindingLayoutPointer m_layout = nullptr;
        uint32_t m_set = 0;
        std::vector<BindingSetItemDesc> m_items;
    };

    class BindingSet : public GraphicObject
    {
    public:
        BindingSet(const BindingSetDesc& _desc);

        const BindingSetDesc& getDesc() const;

    private:
        BindingSetDesc m_desc;
    };

    EGO_POINTER(BindingSet);
}

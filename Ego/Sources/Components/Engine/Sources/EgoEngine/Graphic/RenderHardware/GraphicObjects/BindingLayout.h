#pragma once

#include <cstdint>
#include <vector>

#include "GraphicResource.h"
#include "Shader.h"

namespace ego::gpu
{
    struct PushConstantRangeDesc final
    {
        uint32_t m_offset = 0;
        uint32_t m_size = 0;
        ShaderStageFlag m_stageFlag = ShaderStageFlagNone;
    };

    struct BindingLayoutDesc final
    {
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
}

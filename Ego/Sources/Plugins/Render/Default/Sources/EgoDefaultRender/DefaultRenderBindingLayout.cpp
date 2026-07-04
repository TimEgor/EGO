#include "DefaultRenderBindingLayout.h"

#include "EgoGraphicHardware/GraphicDevice.h"

#include "RenderShaderData.h"

ego::render::RenderBindingLayout ego::render::CreateDefaultRenderBindlessBindingLayout(GraphicDevice& _graphicDevice)
{
    gpu::BindingLayoutDesc bindingLayoutDesc;

    gpu::PushConstantRangeDesc renderBindlessRootConstants;
    renderBindlessRootConstants.m_offset = RenderBindlessRootConstantsOffset;
    renderBindlessRootConstants.m_size = RenderBindlessRootConstantsSize;
    renderBindlessRootConstants.m_stageFlag = RenderBindlessRootConstantsStageFlag;
    bindingLayoutDesc.m_pushConstants.push_back(renderBindlessRootConstants);

    return _graphicDevice.createBindingLayout(bindingLayoutDesc);
}

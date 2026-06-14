#ifndef EGO_DEFAULT_RENDER_GBUFFER_PASS_HLSLI
#define EGO_DEFAULT_RENDER_GBUFFER_PASS_HLSLI

struct CameraShaderData
{
    column_major float4x4 View;
    column_major float4x4 Projection;
    column_major float4x4 ViewProjection;
    float4 Position;
    float4 ScreenSize;
};

struct GBufferRenderData
{
    uint CameraDataIndex;
    uint ObjectDataIndex;
    uint ObjectIndex;
    uint Padding;
};

CameraShaderData LoadCameraShaderData(uint _cameraDataIndex)
{
    ConstantBuffer<CameraShaderData> cameraData = ResourceDescriptorHeap[_cameraDataIndex];

    CameraShaderData result;
    result.View = cameraData.View;
    result.Projection = cameraData.Projection;
    result.ViewProjection = cameraData.ViewProjection;
    result.Position = cameraData.Position;
    result.ScreenSize = cameraData.ScreenSize;
    return result;
}

#endif

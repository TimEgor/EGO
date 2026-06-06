struct VSInput
{
    float3 Position : POSITION;
    float4 Color : COLOR;
};

struct CameraShaderData
{
    column_major float4x4 View;
    column_major float4x4 Projection;
    column_major float4x4 ViewProjection;
    float4 Position;
};

struct VSOutput
{
    float4 Position : SV_POSITION;
    float4 Color : COLOR;
};

cbuffer RenderBindlessRootConstants : register(b0)
{
    uint CameraDataIndex;
    uint ObjectDataIndex;
    uint ObjectIndex;
    uint Padding;
};

VSOutput VSMain(VSInput input)
{
    ConstantBuffer<CameraShaderData> cameraData = ResourceDescriptorHeap[CameraDataIndex];

    VSOutput output;
    output.Position = mul(cameraData.ViewProjection, float4(input.Position, 1.0));
    output.Color = input.Color;
    return output;
}

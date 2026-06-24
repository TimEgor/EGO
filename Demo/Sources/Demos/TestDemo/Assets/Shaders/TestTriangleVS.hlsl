#include "/Shaders/Common/GBufferPass.hlsli"
#include "/Shaders/Common/ObjectTransform.hlsli"

struct VSInput
{
    float3 Position : POSITION;
    float4 Color : COLOR;
};

struct VSOutput
{
    float4 Position : SV_POSITION;
    float4 Color : COLOR;
};

ConstantBuffer<GBufferRenderData> RenderBindless : register(b0);

VSOutput VSMain(VSInput input)
{
    VSOutput output;
    output.Position = TransformObjectToClipPosition(
        RenderBindless.CameraDataIndex,
        RenderBindless.ObjectDataIndex,
        RenderBindless.ObjectIndex,
        input.Position
    );
    output.Color = input.Color;
    return output;
}

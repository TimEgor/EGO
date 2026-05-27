struct VSInput
{
    float3 Position : POSITION;
    float4 Color : COLOR;
};

struct RenderBindlessRootConstants
{
    uint CameraDataIndex;
    uint ObjectDataIndex;
    uint ObjectIndex;
    uint Padding;
};

struct CameraShaderData
{
    column_major float4x4 View;
    column_major float4x4 Projection;
    column_major float4x4 ViewProjection;
    float4 Position;
};

struct ObjectShaderData
{
    column_major float4x4 Model;
    column_major float4x4 ModelViewProjection;
};

struct VSOutput
{
    float4 Position : SV_POSITION;
    float4 Color : COLOR;
};

ConstantBuffer<RenderBindlessRootConstants> RenderBindless : register(b0);

VSOutput VSMain(VSInput input)
{
    ConstantBuffer<CameraShaderData> cameraData = ResourceDescriptorHeap[RenderBindless.CameraDataIndex];
    StructuredBuffer<ObjectShaderData> objectDataBuffer = ResourceDescriptorHeap[RenderBindless.ObjectDataIndex];
    ObjectShaderData objectData = objectDataBuffer[RenderBindless.ObjectIndex];

    VSOutput output;
    output.Position = mul(cameraData.ViewProjection, mul(objectData.Model, float4(input.Position, 1.0)));
    output.Color = input.Color;
    return output;
}

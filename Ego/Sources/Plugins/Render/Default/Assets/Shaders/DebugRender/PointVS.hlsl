struct VSInput
{
    float3 InstancePosition : INST_POSITION;
    float3 InstanceColor : INST_COLOR;
};

struct CameraShaderData
{
    column_major float4x4 View;
    column_major float4x4 Projection;
    column_major float4x4 ViewProjection;
    float4 Position;
    float4 ScreenSize;
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

static const float2 PointVertexOffsets[4] =
{
    float2(0.5, -0.5),
    float2(0.5, 0.5),
    float2(-0.5, -0.5),
    float2(-0.5, 0.5)
};

static const float PointSize = 6.0;

VSOutput VSMain(VSInput input, uint vertexID : SV_VertexID)
{
    ConstantBuffer<CameraShaderData> cameraData = ResourceDescriptorHeap[CameraDataIndex];

    float4 position = mul(cameraData.ViewProjection, float4(input.InstancePosition, 1.0));
    const float2 screenSize = max(cameraData.ScreenSize.xy, float2(1.0, 1.0));
    const float2 pointVertexOffset = PointVertexOffsets[vertexID] * (PointSize * 2.0 / screenSize);

    VSOutput output;
    output.Position = position + float4(pointVertexOffset * position.w, 0.0, 0.0);
    output.Color = float4(input.InstanceColor, 1.0);
    return output;
}

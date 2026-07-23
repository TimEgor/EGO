struct VSInput
{
    float2 Position : POSITION;
    float2 UV : TEXCOORD;
    float4 Color : COLOR;
};

struct VSOutput
{
    float4 Position : SV_POSITION;
    float2 UV : TEXCOORD;
    float4 Color : COLOR;
};

cbuffer GuiConstants : register(b0)
{
    float2 ViewportSize;
    uint TextureIndex;
    uint SamplerIndex;
    uint TextureSamplingMode;
};

VSOutput VSMain(VSInput input)
{
    VSOutput output;
    float2 ndcPosition = float2(input.Position.x / ViewportSize.x * 2.0f - 1.0f, 1.0f - input.Position.y / ViewportSize.y * 2.0f);
    output.Position = float4(ndcPosition, 0.0f, 1.0f);
    output.UV = input.UV;
    output.Color = input.Color;
    return output;
}

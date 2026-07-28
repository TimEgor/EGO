struct PSInput
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

float4 PSMain(PSInput input)
    : SV_TARGET
{
    if (TextureIndex != 0xffffffff)
    {
        Texture2D<float4> texture = ResourceDescriptorHeap[TextureIndex];
        SamplerState textureSampler = SamplerDescriptorHeap[SamplerIndex];
        const float4 textureColor = texture.Sample(textureSampler, input.UV);
        if (TextureSamplingMode == 1)
        {
            return input.Color * textureColor;
        }

        const float textureAlpha = TextureSamplingMode == 2 ? textureColor.a : textureColor.r;
        return float4(input.Color.rgb, input.Color.a * textureAlpha);
    }

    return input.Color;
}

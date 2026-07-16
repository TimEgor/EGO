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
};

float4 PSMain(PSInput input) : SV_TARGET
{
    if (TextureIndex != 0xffffffff)
    {
        Texture2D<float> fontTexture = ResourceDescriptorHeap[TextureIndex];
        SamplerState fontSampler = SamplerDescriptorHeap[SamplerIndex];
        return float4(input.Color.rgb, input.Color.a * fontTexture.Sample(fontSampler, input.UV));
    }

    return input.Color;
}

#include "/Shaders/RayTracing/RayTracingCommon.hlsli"

struct RayPayload
{
    float3 Color;
};

[shader("miss")]
void EGO_RT_MISS_ENTRY_POINT(inout RayPayload _payload)
{
    const float3 rayDirection = normalize(WorldRayDirection());
    const float gradient = saturate(rayDirection.y * 0.5 + 0.5);
    _payload.Color = lerp(float3(0.025, 0.035, 0.055), float3(0.35, 0.55, 0.85), gradient);
}

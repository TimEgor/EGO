#include "/Shaders/RayTracing/RayTracingCommon.hlsli"

struct RayPayload
{
    float3 Color;
};

[shader("closesthit")]
void EGO_RT_CLOSEST_HIT_ENTRY_POINT(
    inout RayPayload _payload,
    in BuiltInTriangleIntersectionAttributes _attributes
)
{
    const float3 barycentrics = float3(
        1.0 - _attributes.barycentrics.x - _attributes.barycentrics.y,
        _attributes.barycentrics.x,
        _attributes.barycentrics.y
    );
    const float instanceTint = frac((float)InstanceID() * 0.61803398875);
    const float3 tint = lerp(float3(0.85, 0.55, 0.25), float3(0.25, 0.75, 0.95), instanceTint);
    _payload.Color = saturate((barycentrics * 0.75 + 0.25) * tint);
}

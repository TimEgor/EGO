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
    _payload.Color = float3(0.05, 0.65, 1.0);
}

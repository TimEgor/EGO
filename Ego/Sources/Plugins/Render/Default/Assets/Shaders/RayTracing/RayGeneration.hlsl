#include "/Shaders/Common/GBufferPass.hlsli"
#include "/Shaders/RayTracing/RayTracingCommon.hlsli"

struct RayPayload
{
    float3 Color;
};

float3 UnprojectClipPosition(float4x4 _inverseViewProjection, float2 _clipPosition, float _clipDepth)
{
    float4 worldPosition = mul(_inverseViewProjection, float4(_clipPosition, _clipDepth, 1.0));
    worldPosition.xyz /= worldPosition.w;
    return worldPosition.xyz;
}

cbuffer RenderBindlessRootConstants : register(b0)
{
    uint CameraDataIndex;
    uint SceneDataIndex;
    uint OutputDataIndex;
    uint Padding;
};

[shader("raygeneration")]
void EGO_RT_RAY_GENERATION_ENTRY_POINT()
{
    const uint2 pixelIndex = DispatchRaysIndex().xy;
    const CameraShaderData cameraData = LoadCameraShaderData(CameraDataIndex);
    RWTexture2D<float4> outputTexture = ResourceDescriptorHeap[OutputDataIndex];
    RaytracingAccelerationStructure scene = ResourceDescriptorHeap[SceneDataIndex];

    const float2 screenSize = max(cameraData.ScreenSize.xy, float2(1.0, 1.0));
    float2 clipPosition = ((float2(pixelIndex) + 0.5) / screenSize) * 2.0 - 1.0;
    clipPosition.y = -clipPosition.y;

    const float3 rayOrigin = UnprojectClipPosition(cameraData.InverseViewProjection, clipPosition, 0.0);
    const float3 rayTarget = UnprojectClipPosition(cameraData.InverseViewProjection, clipPosition, 1.0);

    RayDesc rayDesc;
    rayDesc.Origin = rayOrigin;
    rayDesc.Direction = normalize(rayTarget - rayOrigin);
    rayDesc.TMin = 0.001;
    rayDesc.TMax = 100000.0;

    RayPayload payload;
    payload.Color = float3(0.0, 0.0, 0.0);

    TraceRay(
        scene,
        RAY_FLAG_FORCE_OPAQUE,
        0xff,
        0,
        1,
        0,
        rayDesc,
        payload
    );

    outputTexture[pixelIndex] = float4(payload.Color, 1.0);
}

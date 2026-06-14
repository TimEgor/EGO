#ifndef EGO_DEFAULT_RENDER_OBJECT_TRANSFORM_HLSLI
#define EGO_DEFAULT_RENDER_OBJECT_TRANSFORM_HLSLI

#include "/Assets/Shaders/Common/GBufferPass.hlsli"

struct ObjectShaderData
{
    column_major float4x4 Model;
};

ObjectShaderData LoadObjectShaderData(uint _objectDataIndex, uint _objectIndex)
{
    StructuredBuffer<ObjectShaderData> objectDataBuffer = ResourceDescriptorHeap[_objectDataIndex];
    return objectDataBuffer[_objectIndex];
}

float4 TransformObjectToWorldPosition(ObjectShaderData _objectData, float3 _objectPosition)
{
    return mul(_objectData.Model, float4(_objectPosition, 1.0));
}

float4 TransformWorldToClipPosition(CameraShaderData _cameraData, float4 _worldPosition)
{
    return mul(_cameraData.ViewProjection, _worldPosition);
}

float4 TransformObjectToClipPosition(
    CameraShaderData _cameraData,
    ObjectShaderData _objectData,
    float3 _objectPosition
)
{
    return TransformWorldToClipPosition(_cameraData, TransformObjectToWorldPosition(_objectData, _objectPosition));
}

float4 TransformObjectToClipPosition(
    uint _cameraDataIndex,
    uint _objectDataIndex,
    uint _objectIndex,
    float3 _objectPosition
)
{
    return TransformObjectToClipPosition(
        LoadCameraShaderData(_cameraDataIndex),
        LoadObjectShaderData(_objectDataIndex, _objectIndex),
        _objectPosition
    );
}

#endif

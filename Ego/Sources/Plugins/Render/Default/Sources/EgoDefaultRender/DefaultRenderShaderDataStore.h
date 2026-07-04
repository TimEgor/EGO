#pragma once

#include <cstdint>

#include "EgoECS/Entity.h"

#include "EgoMath/Matrix.h"

#include "EgoEngine/Graphic/Render/RenderGpuObject.h"
#include "EgoGraphicHardware/GraphicDevice.h"

#include "DefaultRenderScene.h"

namespace ego
{
    class Level;
}

namespace ego::render
{
    class DefaultRenderShaderDataStore final
    {
    public:
        DefaultRenderShaderDataStore() = default;

        bool prepare(GraphicDevice& _graphicDevice, Level& _level, ecs::Entity _cameraEntity, DefaultRenderScene& _scene, const gpu::Texture2DSize& _resolution);
        void release();
        void clearResources();

        const RenderBufferView& getCameraShaderDataView() const;
        const RenderBufferView& getObjectShaderDataView() const;
        const ComputeMatrix4x4& getCameraViewProjectionMatrix() const;

    private:
        bool prepareCameraShaderData(GraphicDevice& _graphicDevice, Level& _level, ecs::Entity _cameraEntity, const gpu::Texture2DSize& _resolution);
        bool prepareObjectShaderData(GraphicDevice& _graphicDevice, DefaultRenderScene& _scene);

        RenderBuffer m_cameraShaderDataBuffer = nullptr;
        RenderBufferView m_cameraShaderDataView = nullptr;
        RenderBuffer m_objectShaderDataBuffer = nullptr;
        RenderBufferView m_objectShaderDataView = nullptr;
        ComputeMatrix4x4 m_cameraViewProjectionMatrix = ComputeMatrix4x4Identity;
        uint32_t m_objectShaderDataCapacity = 0;
    };
} // namespace ego::render

#pragma once

#include <cstdint>

#include "EgoGraphicHardware/GraphicDevice.h"

#include "EgoEngine/Graphic/SceneRender/RenderGpuObject.h"

#include "DefaultRenderScene.h"

namespace ego::render
{
    class RenderCamera;

    class DefaultRenderShaderDataStore final
    {
    public:
        DefaultRenderShaderDataStore() = default;

        bool prepare(GraphicDevice& _graphicDevice, const RenderCamera& _camera, DefaultRenderScene& _scene, const gpu::Texture2DSize& _resolution);
        void release();

        const RenderBufferView& getCameraShaderDataView() const;
        const RenderBufferView& getObjectShaderDataView() const;

    private:
        bool prepareCameraShaderData(GraphicDevice& _graphicDevice, const RenderCamera& _camera, const gpu::Texture2DSize& _resolution);
        bool prepareObjectShaderData(GraphicDevice& _graphicDevice, DefaultRenderScene& _scene);

        RenderBuffer m_cameraShaderDataBuffer = nullptr;
        RenderBufferView m_cameraShaderDataView = nullptr;
        RenderBuffer m_objectShaderDataBuffer = nullptr;
        RenderBufferView m_objectShaderDataView = nullptr;
        uint32_t m_objectShaderDataCapacity = 0;
    };
} // namespace ego::render

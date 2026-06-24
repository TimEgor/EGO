#pragma once

#include <cstdint>

#include <wrl/client.h>

#include "Common/D3D12Utils.h"

#include "EgoEngine/Graphic/RenderHardware/GraphicDevice.h"

namespace ego::gpu::d3d12
{
    class D3D12DeviceContext final
    {
    public:
        D3D12DeviceContext() = default;

        bool init(const GraphicDevice::InitParams& _params, uint32_t _bindlessResourceDescriptorCount, uint32_t _bindlessSamplerDescriptorCount);
        void release();

        IDXGIFactory6* getFactory() const;
        IDXGIAdapter1* getAdapter() const;
        ID3D12Device5* getDevice() const;
        const GraphicDevice::Capabilities& getCapabilities() const;

    private:
        bool initializeFactory(const GraphicDevice::InitParams& _params);
        bool initializeAdapter();
        bool initializeDevice();
        void initializeCapabilities();
        static bool SupportsRayTracing(ID3D12Device* _device);

        Microsoft::WRL::ComPtr<IDXGIFactory6> m_factory;
        Microsoft::WRL::ComPtr<IDXGIAdapter1> m_adapter;
        Microsoft::WRL::ComPtr<ID3D12Device5> m_device;
        GraphicDevice::Capabilities m_capabilities;
        uint32_t m_bindlessResourceDescriptorCount = 0;
        uint32_t m_bindlessSamplerDescriptorCount = 0;
    };
} // namespace ego::gpu::d3d12

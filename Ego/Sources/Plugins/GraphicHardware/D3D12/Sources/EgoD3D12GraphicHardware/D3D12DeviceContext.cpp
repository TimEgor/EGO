#include "D3D12DeviceContext.h"

bool ego::gpu::d3d12::D3D12DeviceContext::init(const GraphicDevice::InitData& _initData, uint32_t _bindlessResourceDescriptorCount, uint32_t _bindlessSamplerDescriptorCount)
{
    m_bindlessResourceDescriptorCount = _bindlessResourceDescriptorCount;
    m_bindlessSamplerDescriptorCount = _bindlessSamplerDescriptorCount;

    if (!initializeFactory(_initData) || !initializeAdapter() || !initializeDevice())
    {
        release();
        return false;
    }

    return true;
}

void ego::gpu::d3d12::D3D12DeviceContext::release()
{
    m_device.Reset();
    m_adapter.Reset();
    m_factory.Reset();
    m_capabilities = GraphicDevice::Capabilities();
    m_bindlessResourceDescriptorCount = 0;
    m_bindlessSamplerDescriptorCount = 0;
}

IDXGIFactory6* ego::gpu::d3d12::D3D12DeviceContext::getFactory() const
{
    return m_factory.Get();
}

IDXGIAdapter1* ego::gpu::d3d12::D3D12DeviceContext::getAdapter() const
{
    return m_adapter.Get();
}

ID3D12Device5* ego::gpu::d3d12::D3D12DeviceContext::getDevice() const
{
    return m_device.Get();
}

const ego::GraphicDevice::Capabilities& ego::gpu::d3d12::D3D12DeviceContext::getCapabilities() const
{
    return m_capabilities;
}

bool ego::gpu::d3d12::D3D12DeviceContext::initializeFactory(const GraphicDevice::InitData& _initData)
{
    UINT factoryFlags = 0;

    if (_initData.m_debugEnable)
    {
        Microsoft::WRL::ComPtr<ID3D12Debug1> debugController;
        if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
        {
            debugController->EnableDebugLayer();
            debugController->SetEnableGPUBasedValidation(_initData.m_gpuValidation);
            factoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
        }
    }

    return SUCCEEDED(CreateDXGIFactory2(factoryFlags, IID_PPV_ARGS(&m_factory)));
}

bool ego::gpu::d3d12::D3D12DeviceContext::initializeAdapter()
{
    for (UINT adapterIndex = 0;; ++adapterIndex)
    {
        Microsoft::WRL::ComPtr<IDXGIAdapter1> adapter;
        if (FAILED(m_factory->EnumAdapterByGpuPreference(adapterIndex, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&adapter))))
        {
            break;
        }

        DXGI_ADAPTER_DESC1 adapterDesc = {};
        adapter->GetDesc1(&adapterDesc);
        if (adapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
        {
            continue;
        }

        Microsoft::WRL::ComPtr<ID3D12Device5> device;
        if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&device))) && SupportsRayTracing(device.Get()))
        {
            m_adapter = adapter;
            return true;
        }
    }

    return false;
}

bool ego::gpu::d3d12::D3D12DeviceContext::initializeDevice()
{
    if (FAILED(D3D12CreateDevice(m_adapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&m_device))))
    {
        return false;
    }

    initializeCapabilities();
    if (!m_capabilities.m_supportsRayTracing)
    {
        m_device.Reset();
        return false;
    }

    return true;
}

void ego::gpu::d3d12::D3D12DeviceContext::initializeCapabilities()
{
    m_capabilities.m_maxColorAttachments = D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT;
    m_capabilities.m_supportsPresentation = true;
    m_capabilities.m_supportsIndependentComputeQueue = true;
    m_capabilities.m_supportsSamplerAnisotropy = true;
    m_capabilities.m_supportsMeshShaders = false;
    m_capabilities.m_supportsRayTracing = false;
    m_capabilities.m_supportsBindlessResources = false;
    m_capabilities.m_bindlessResourceDescriptorCount = m_bindlessResourceDescriptorCount;
    m_capabilities.m_bindlessSamplerDescriptorCount = m_bindlessSamplerDescriptorCount;

    bool supportsResourceBindingTier = false;
    D3D12_FEATURE_DATA_D3D12_OPTIONS options = {};
    if (SUCCEEDED(m_device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS, &options, sizeof(options))))
    {
        supportsResourceBindingTier = options.ResourceBindingTier >= D3D12_RESOURCE_BINDING_TIER_2;
    }

    bool supportsDirectHeapIndexing = false;
    D3D12_FEATURE_DATA_SHADER_MODEL shaderModel = {};
    shaderModel.HighestShaderModel = D3D_SHADER_MODEL_6_6;
    if (SUCCEEDED(m_device->CheckFeatureSupport(D3D12_FEATURE_SHADER_MODEL, &shaderModel, sizeof(shaderModel))))
    {
        supportsDirectHeapIndexing = shaderModel.HighestShaderModel >= D3D_SHADER_MODEL_6_6;
    }

    m_capabilities.m_supportsBindlessResources = supportsResourceBindingTier && supportsDirectHeapIndexing;

    D3D12_FEATURE_DATA_D3D12_OPTIONS7 options7 = {};
    if (SUCCEEDED(m_device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS7, &options7, sizeof(options7))))
    {
        m_capabilities.m_supportsMeshShaders = options7.MeshShaderTier != D3D12_MESH_SHADER_TIER_NOT_SUPPORTED;
    }

    m_capabilities.m_supportsRayTracing = SupportsRayTracing(m_device.Get());
}

bool ego::gpu::d3d12::D3D12DeviceContext::SupportsRayTracing(ID3D12Device* _device)
{
    if (!_device)
    {
        return false;
    }

    D3D12_FEATURE_DATA_D3D12_OPTIONS5 options5 = {};
    return SUCCEEDED(_device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &options5, sizeof(options5))) && options5.RaytracingTier != D3D12_RAYTRACING_TIER_NOT_SUPPORTED;
}

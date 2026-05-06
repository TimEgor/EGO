#include "TestCubeRenderer.h"

#include "EgoEngine/Platform/Window/Window.h"

#if defined(_WIN32)

#include <array>
#include <climits>
#include <cstring>
#include <cmath>
#include <iterator>

#include <Windows.h>
#include <d3d12.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <dxgi1_6.h>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

namespace
{
    constexpr uint32_t FrameCount = 2;

    struct Vertex final
    {
        float m_position[3];
        float m_color[3];
    };

    struct TransformConstants final
    {
        DirectX::XMFLOAT4X4 m_mvp;
    };

    constexpr Vertex CubeVertices[] =
    {
        {{-1.0f, -1.0f, -1.0f}, {0.8f, 0.1f, 0.1f}},
        {{-1.0f,  1.0f, -1.0f}, {0.8f, 0.8f, 0.1f}},
        {{ 1.0f,  1.0f, -1.0f}, {0.1f, 0.8f, 0.1f}},
        {{ 1.0f, -1.0f, -1.0f}, {0.1f, 0.6f, 0.8f}},
        {{-1.0f, -1.0f,  1.0f}, {0.8f, 0.1f, 0.8f}},
        {{-1.0f,  1.0f,  1.0f}, {0.4f, 0.4f, 1.0f}},
        {{ 1.0f,  1.0f,  1.0f}, {0.1f, 0.9f, 0.9f}},
        {{ 1.0f, -1.0f,  1.0f}, {1.0f, 0.5f, 0.1f}}
    };

    constexpr uint16_t CubeIndices[] =
    {
        0, 1, 2, 0, 2, 3,
        4, 6, 5, 4, 7, 6,
        4, 5, 1, 4, 1, 0,
        3, 2, 6, 3, 6, 7,
        1, 5, 6, 1, 6, 2,
        4, 0, 3, 4, 3, 7
    };

    const char* ShaderSource = R"(
        struct VSInput
        {
            float3 position : POSITION;
            float3 color : COLOR;
        };

        struct VSOutput
        {
            float4 position : SV_POSITION;
            float3 color : COLOR;
        };

        cbuffer Transform : register(b0)
        {
            row_major float4x4 mvp;
        };

        VSOutput VSMain(VSInput input)
        {
            VSOutput output;
            output.position = mul(float4(input.position, 1.0f), mvp);
            output.color = input.color;
            return output;
        }

        float4 PSMain(VSOutput input) : SV_TARGET
        {
            return float4(input.color, 1.0f);
        }
    )";

    bool CreateUploadBuffer(
        ID3D12Device* _device,
        const void* _data,
        uint64_t _size,
        ID3D12Resource** _resource
    )
    {
        D3D12_HEAP_PROPERTIES heapProperties = {};
        heapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;

        D3D12_RESOURCE_DESC resourceDesc = {};
        resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        resourceDesc.Width = _size;
        resourceDesc.Height = 1;
        resourceDesc.DepthOrArraySize = 1;
        resourceDesc.MipLevels = 1;
        resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
        resourceDesc.SampleDesc.Count = 1;
        resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

        ComPtr<ID3D12Resource> resource;
        if (FAILED(
            _device->CreateCommittedResource(
                &heapProperties,
                D3D12_HEAP_FLAG_NONE,
                &resourceDesc,
                D3D12_RESOURCE_STATE_GENERIC_READ,
                nullptr,
                IID_PPV_ARGS(&resource)
            )
        ))
        {
            return false;
        }

        void* mappedData = nullptr;
        D3D12_RANGE readRange = {};
        if (FAILED(resource->Map(0, &readRange, &mappedData)))
        {
            return false;
        }

        memcpy(mappedData, _data, static_cast<size_t>(_size));

        D3D12_RANGE writtenRange = {};
        writtenRange.Begin = 0;
        writtenRange.End = static_cast<SIZE_T>(_size);
        resource->Unmap(0, &writtenRange);

        *_resource = resource.Detach();
        return true;
    }

    bool CompileShader(const char* _entryPoint, const char* _target, ID3DBlob** _shader)
    {
        UINT compileFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(EGO_CONFIG_DEBUG)
        compileFlags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

        ComPtr<ID3DBlob> shader;
        ComPtr<ID3DBlob> errors;
        const HRESULT result = D3DCompile(
            ShaderSource,
            strlen(ShaderSource),
            nullptr,
            nullptr,
            nullptr,
            _entryPoint,
            _target,
            compileFlags,
            0,
            &shader,
            &errors
        );

        if (FAILED(result))
        {
            OutputDebugStringA(errors ? static_cast<const char*>(errors->GetBufferPointer()) : "Shader compile failed");
            return false;
        }

        *_shader = shader.Detach();
        return true;
    }

    D3D12_RESOURCE_BARRIER TransitionBarrier(
        ID3D12Resource* _resource,
        D3D12_RESOURCE_STATES _before,
        D3D12_RESOURCE_STATES _after
    )
    {
        D3D12_RESOURCE_BARRIER barrier = {};
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrier.Transition.pResource = _resource;
        barrier.Transition.StateBefore = _before;
        barrier.Transition.StateAfter = _after;
        barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        return barrier;
    }
}

struct ego::render::TestCubeRenderer::Impl final
{
    HWND m_windowHandle = nullptr;
    uint32_t m_width = 0;
    uint32_t m_height = 0;
    uint32_t m_frameIndex = 0;
    uint64_t m_fenceValue = 0;
    float m_rotation = 0.0f;

    ComPtr<IDXGIFactory6> m_factory;
    ComPtr<ID3D12Device> m_device;
    ComPtr<ID3D12CommandQueue> m_commandQueue;
    ComPtr<IDXGISwapChain3> m_swapChain;
    ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
    ComPtr<ID3D12DescriptorHeap> m_dsvHeap;
    ComPtr<ID3D12Resource> m_renderTargets[FrameCount];
    ComPtr<ID3D12Resource> m_depthStencil;
    ComPtr<ID3D12CommandAllocator> m_commandAllocators[FrameCount];
    ComPtr<ID3D12GraphicsCommandList> m_commandList;
    ComPtr<ID3D12Fence> m_fence;
    HANDLE m_fenceEvent = nullptr;
    uint32_t m_rtvDescriptorSize = 0;

    ComPtr<ID3D12RootSignature> m_rootSignature;
    ComPtr<ID3D12PipelineState> m_pipelineState;
    ComPtr<ID3D12Resource> m_vertexBuffer;
    ComPtr<ID3D12Resource> m_indexBuffer;
    D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView = {};
    D3D12_INDEX_BUFFER_VIEW m_indexBufferView = {};
    D3D12_VIEWPORT m_viewport = {};
    D3D12_RECT m_scissorRect = {};

    bool init(Window& _window)
    {
        m_windowHandle = static_cast<HWND>(_window.getNativeHandle());
        if (!m_windowHandle)
        {
            return false;
        }

        const WindowSize& clientSize = _window.getClientAreaSize();
        m_width = clientSize.m_x ? clientSize.m_x : 1;
        m_height = clientSize.m_y ? clientSize.m_y : 1;

        return createDevice() &&
            createSwapChain() &&
            createFrameResources() &&
            createPipeline() &&
            createGeometry() &&
            createFence();
    }

    void release()
    {
        waitForGpu();

        if (m_fenceEvent)
        {
            CloseHandle(m_fenceEvent);
            m_fenceEvent = nullptr;
        }
    }

    void render(float _deltaTime)
    {
        if (!m_swapChain)
        {
            return;
        }

        m_rotation += _deltaTime;

        ID3D12CommandAllocator* allocator = m_commandAllocators[m_frameIndex].Get();
        allocator->Reset();
        m_commandList->Reset(allocator, m_pipelineState.Get());

        D3D12_RESOURCE_BARRIER beginBarrier = TransitionBarrier(
            m_renderTargets[m_frameIndex].Get(),
            D3D12_RESOURCE_STATE_PRESENT,
            D3D12_RESOURCE_STATE_RENDER_TARGET
        );
        m_commandList->ResourceBarrier(1, &beginBarrier);

        const D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = getCurrentRtvHandle();
        const D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = m_dsvHeap->GetCPUDescriptorHandleForHeapStart();
        constexpr float clearColor[] = {0.05f, 0.08f, 0.12f, 1.0f};
        m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
        m_commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
        m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

        m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());
        m_commandList->RSSetViewports(1, &m_viewport);
        m_commandList->RSSetScissorRects(1, &m_scissorRect);
        m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        m_commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
        m_commandList->IASetIndexBuffer(&m_indexBufferView);

        const TransformConstants constants = buildTransformConstants();
        m_commandList->SetGraphicsRoot32BitConstants(
            0,
            sizeof(TransformConstants) / sizeof(uint32_t),
            &constants,
            0
        );
        m_commandList->DrawIndexedInstanced(static_cast<UINT>(std::size(CubeIndices)), 1, 0, 0, 0);

        D3D12_RESOURCE_BARRIER endBarrier = TransitionBarrier(
            m_renderTargets[m_frameIndex].Get(),
            D3D12_RESOURCE_STATE_RENDER_TARGET,
            D3D12_RESOURCE_STATE_PRESENT
        );
        m_commandList->ResourceBarrier(1, &endBarrier);
        m_commandList->Close();

        ID3D12CommandList* commandLists[] = {m_commandList.Get()};
        m_commandQueue->ExecuteCommandLists(1, commandLists);
        m_swapChain->Present(1, 0);
        waitForGpu();

        m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
    }

    bool createDevice()
    {
        if (FAILED(CreateDXGIFactory2(0, IID_PPV_ARGS(&m_factory))))
        {
            return false;
        }

        if (FAILED(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&m_device))))
        {
            return false;
        }

        D3D12_COMMAND_QUEUE_DESC queueDesc = {};
        queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        queueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
        return SUCCEEDED(m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue)));
    }

    bool createSwapChain()
    {
        DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
        swapChainDesc.Width = m_width;
        swapChainDesc.Height = m_height;
        swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        swapChainDesc.SampleDesc.Count = 1;
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.BufferCount = FrameCount;
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

        ComPtr<IDXGISwapChain1> swapChain;
        if (FAILED(
            m_factory->CreateSwapChainForHwnd(
                m_commandQueue.Get(),
                m_windowHandle,
                &swapChainDesc,
                nullptr,
                nullptr,
                &swapChain
            )
        ))
        {
            return false;
        }

        m_factory->MakeWindowAssociation(m_windowHandle, DXGI_MWA_NO_ALT_ENTER);
        if (FAILED(swapChain.As(&m_swapChain)))
        {
            return false;
        }

        m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
        return true;
    }

    bool createFrameResources()
    {
        D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
        rtvHeapDesc.NumDescriptors = FrameCount;
        rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        if (FAILED(m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap))))
        {
            return false;
        }

        m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

        D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_rtvHeap->GetCPUDescriptorHandleForHeapStart();
        for (uint32_t i = 0; i < FrameCount; ++i)
        {
            if (FAILED(m_swapChain->GetBuffer(i, IID_PPV_ARGS(&m_renderTargets[i]))))
            {
                return false;
            }

            m_device->CreateRenderTargetView(m_renderTargets[i].Get(), nullptr, rtvHandle);
            rtvHandle.ptr += m_rtvDescriptorSize;

            if (FAILED(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocators[i]))))
            {
                return false;
            }
        }

        D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
        dsvHeapDesc.NumDescriptors = 1;
        dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
        if (FAILED(m_device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_dsvHeap))))
        {
            return false;
        }

        D3D12_HEAP_PROPERTIES depthHeapProperties = {};
        depthHeapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;

        D3D12_RESOURCE_DESC depthDesc = {};
        depthDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        depthDesc.Width = m_width;
        depthDesc.Height = m_height;
        depthDesc.DepthOrArraySize = 1;
        depthDesc.MipLevels = 1;
        depthDesc.Format = DXGI_FORMAT_D32_FLOAT;
        depthDesc.SampleDesc.Count = 1;
        depthDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

        D3D12_CLEAR_VALUE depthClearValue = {};
        depthClearValue.Format = DXGI_FORMAT_D32_FLOAT;
        depthClearValue.DepthStencil.Depth = 1.0f;

        if (FAILED(
            m_device->CreateCommittedResource(
                &depthHeapProperties,
                D3D12_HEAP_FLAG_NONE,
                &depthDesc,
                D3D12_RESOURCE_STATE_DEPTH_WRITE,
                &depthClearValue,
                IID_PPV_ARGS(&m_depthStencil)
            )
        ))
        {
            return false;
        }

        m_device->CreateDepthStencilView(m_depthStencil.Get(), nullptr, m_dsvHeap->GetCPUDescriptorHandleForHeapStart());

        if (FAILED(
            m_device->CreateCommandList(
                0,
                D3D12_COMMAND_LIST_TYPE_DIRECT,
                m_commandAllocators[m_frameIndex].Get(),
                nullptr,
                IID_PPV_ARGS(&m_commandList)
            )
        ))
        {
            return false;
        }

        m_commandList->Close();

        m_viewport = {0.0f, 0.0f, static_cast<float>(m_width), static_cast<float>(m_height), 0.0f, 1.0f};
        m_scissorRect = {0, 0, static_cast<LONG>(m_width), static_cast<LONG>(m_height)};
        return true;
    }

    bool createPipeline()
    {
        D3D12_ROOT_PARAMETER rootParameter = {};
        rootParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
        rootParameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
        rootParameter.Constants.ShaderRegister = 0;
        rootParameter.Constants.RegisterSpace = 0;
        rootParameter.Constants.Num32BitValues = sizeof(TransformConstants) / sizeof(uint32_t);

        D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
        rootSignatureDesc.NumParameters = 1;
        rootSignatureDesc.pParameters = &rootParameter;
        rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

        ComPtr<ID3DBlob> serializedRootSignature;
        ComPtr<ID3DBlob> rootSignatureErrors;
        if (FAILED(
            D3D12SerializeRootSignature(
                &rootSignatureDesc,
                D3D_ROOT_SIGNATURE_VERSION_1,
                &serializedRootSignature,
                &rootSignatureErrors
            )
        ))
        {
            return false;
        }

        if (FAILED(
            m_device->CreateRootSignature(
                0,
                serializedRootSignature->GetBufferPointer(),
                serializedRootSignature->GetBufferSize(),
                IID_PPV_ARGS(&m_rootSignature)
            )
        ))
        {
            return false;
        }

        ComPtr<ID3DBlob> vertexShader;
        ComPtr<ID3DBlob> pixelShader;
        if (!CompileShader("VSMain", "vs_5_1", &vertexShader) ||
            !CompileShader("PSMain", "ps_5_1", &pixelShader))
        {
            return false;
        }

        D3D12_INPUT_ELEMENT_DESC inputElements[] =
        {
            {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
            {"COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
        };

        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
        psoDesc.InputLayout = {inputElements, static_cast<UINT>(std::size(inputElements))};
        psoDesc.pRootSignature = m_rootSignature.Get();
        psoDesc.VS = {vertexShader->GetBufferPointer(), vertexShader->GetBufferSize()};
        psoDesc.PS = {pixelShader->GetBufferPointer(), pixelShader->GetBufferSize()};
        psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
        psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
        psoDesc.RasterizerState.DepthClipEnable = TRUE;
        psoDesc.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
        psoDesc.DepthStencilState.DepthEnable = TRUE;
        psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
        psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
        psoDesc.SampleMask = UINT_MAX;
        psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        psoDesc.NumRenderTargets = 1;
        psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
        psoDesc.SampleDesc.Count = 1;

        return SUCCEEDED(m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState)));
    }

    bool createGeometry()
    {
        const uint64_t vertexBufferSize = sizeof(CubeVertices);
        const uint64_t indexBufferSize = sizeof(CubeIndices);

        if (!CreateUploadBuffer(m_device.Get(), CubeVertices, vertexBufferSize, &m_vertexBuffer) ||
            !CreateUploadBuffer(m_device.Get(), CubeIndices, indexBufferSize, &m_indexBuffer))
        {
            return false;
        }

        m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
        m_vertexBufferView.StrideInBytes = sizeof(Vertex);
        m_vertexBufferView.SizeInBytes = static_cast<UINT>(vertexBufferSize);

        m_indexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
        m_indexBufferView.Format = DXGI_FORMAT_R16_UINT;
        m_indexBufferView.SizeInBytes = static_cast<UINT>(indexBufferSize);
        return true;
    }

    bool createFence()
    {
        if (FAILED(m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence))))
        {
            return false;
        }

        m_fenceValue = 1;
        m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        return m_fenceEvent != nullptr;
    }

    void waitForGpu()
    {
        if (!m_commandQueue || !m_fence)
        {
            return;
        }

        const uint64_t fenceValue = m_fenceValue;
        m_commandQueue->Signal(m_fence.Get(), fenceValue);
        ++m_fenceValue;

        if (m_fence->GetCompletedValue() < fenceValue)
        {
            m_fence->SetEventOnCompletion(fenceValue, m_fenceEvent);
            WaitForSingleObject(m_fenceEvent, INFINITE);
        }
    }

    D3D12_CPU_DESCRIPTOR_HANDLE getCurrentRtvHandle() const
    {
        D3D12_CPU_DESCRIPTOR_HANDLE handle = m_rtvHeap->GetCPUDescriptorHandleForHeapStart();
        handle.ptr += static_cast<SIZE_T>(m_frameIndex * m_rtvDescriptorSize);
        return handle;
    }

    TransformConstants buildTransformConstants() const
    {
        using namespace DirectX;

        const float aspect = m_height ? static_cast<float>(m_width) / static_cast<float>(m_height) : 1.0f;
        const XMMATRIX world = XMMatrixRotationX(m_rotation * 0.65f) * XMMatrixRotationY(m_rotation);
        const XMMATRIX view = XMMatrixLookAtLH(
            XMVectorSet(0.0f, 1.0f, -5.0f, 1.0f),
            XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f),
            XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)
        );
        const XMMATRIX projection = XMMatrixPerspectiveFovLH(XM_PIDIV4, aspect, 0.1f, 100.0f);

        TransformConstants constants;
        XMStoreFloat4x4(&constants.m_mvp, world * view * projection);
        return constants;
    }
};

#else

struct ego::render::TestCubeRenderer::Impl final
{
    bool init(Window&) { return false; }
    void release() {}
    void render(float) {}
};

#endif

ego::render::TestCubeRenderer::~TestCubeRenderer()
{
    release();
}

bool ego::render::TestCubeRenderer::init(Window& _window)
{
    release();

    m_impl = new Impl();
    if (!m_impl->init(_window))
    {
        release();
        return false;
    }

    return true;
}

void ego::render::TestCubeRenderer::release()
{
    if (!m_impl)
    {
        return;
    }

    m_impl->release();
    delete m_impl;
    m_impl = nullptr;
}

void ego::render::TestCubeRenderer::render(float _deltaTime)
{
    if (m_impl)
    {
        m_impl->render(_deltaTime);
    }
}

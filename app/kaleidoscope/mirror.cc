#include <concepts>
#include <stdexcept>
#include <ranges>
#include <array>

#define NOMINMAX
#include <Windows.h>
#include <D3d12SDKLayers.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
#include <comdef.h>

#include "tool.h"

// Run "build" to generate header files.
//
// About more details:
// https://learn.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl-part1
//
namespace compiled_shader
{
#include "pixel_shader.h"
#include "vertex_shader.h"
}

// Rename namespace
namespace wrl = Microsoft::WRL;

// Error handle
namespace must
{
    auto constexpr succeed = [](HRESULT hr)
    {
        if (SUCCEEDED(hr))
            return hr;

        // About error message:
        // https://stackoverflow.com/a/7008111
        auto err = _com_error(hr);
        auto desc = err.ErrorMessage();
        OutputDebugString(desc);

        throw err;
    };

    auto constexpr succeed_with = [](ID3DBlob & message)
    {
        return [&](HRESULT hr)
        {
            if (SUCCEEDED(hr))
                return hr;

            OutputDebugString(static_cast<char const *>(message.GetBufferPointer()));
            throw _com_error(hr);
        };
    };
}

// Builder
namespace create
{
    using wrl::ComPtr;

    auto static d3d12_device(ComPtr<IDXGIFactory4> const & factory, ComPtr<ID3D12Device> & device) -> HRESULT
    {
        // Create adapter and device
        auto adapter = ComPtr<IDXGIAdapter1>();
        for (auto index = UINT{}; ; ++index)
        {
            // Note for ComPtr: "operator &" == "ReleaseAndGetAddressOf()"
            if (auto hr = factory->EnumAdapters1(index, &adapter); FAILED(hr))
                return hr;

            auto desc = DXGI_ADAPTER_DESC1{};
            if (auto hr = adapter->GetDesc1(&desc); FAILED(hr))
                return hr;

            if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
                continue;

            if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&device))))
                break;
        }

        // Common HRESULT Values:
        // https://learn.microsoft.com/en-us/windows/win32/seccrypto/common-hresult-values
        return S_OK;
    }
}

struct core
{
    explicit core(HWND window)
    {
        using namespace aux;
        using wrl::ComPtr;

        /////////////////////////////////////////////////////////////////////
        /// Setup environment

        // Create debug layer.
        // Note: must enable it before creating our device.
        if constexpr (env::is_debug())
        {
            auto base_controller = ComPtr<ID3D12Debug>();
            D3D12GetDebugInterface(IID_PPV_ARGS(&base_controller)) >> must::succeed;

            auto controller = ComPtr<ID3D12Debug1>();
            base_controller.As(&controller) >> must::succeed;
            // use "As" instead of "QueryInterface":
            // https://github.com/Microsoft/DirectXTK/wiki/ComPtr#obtaining-new-interfaces

            controller->EnableDebugLayer();
            controller->SetEnableGPUBasedValidation(true);
        }

        // Create DXGI factory
        auto factory = ComPtr<IDXGIFactory4>();
        {
            auto flag = env::is_debug() ? DXGI_CREATE_FACTORY_DEBUG : UINT{};
            CreateDXGIFactory2(flag, IID_PPV_ARGS(&factory)) >> must::succeed;
        }

        // Create device from a factory and an (internal created) adapter
        {
            create::d3d12_device(factory, device) >> must::succeed;
            if constexpr (env::is_debug())
                device.As(&debug_device) >> must::succeed;
        }

        // Create command queue and command allocator
        {
            auto desc = D3D12_COMMAND_QUEUE_DESC{};
            desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
            desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
            device->CreateCommandQueue(&desc, IID_PPV_ARGS(&command_queue)) >> must::succeed;
            device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&command_allocator)) >> must::succeed;
        }

        // Create swap_chain
        {
            auto normal = DXGI_SWAP_CHAIN_DESC1{};
            normal.BufferCount = static_cast<UINT>(render_targets.size());
            normal.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            normal.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
            normal.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
            normal.SampleDesc.Count = 1;

            auto fullscreen = DXGI_SWAP_CHAIN_FULLSCREEN_DESC{};
            fullscreen.Windowed = true;

            auto base = ComPtr<IDXGISwapChain1>();
            factory->CreateSwapChainForHwnd(command_queue.Get(), window, &normal, &fullscreen, nullptr, &base) >> must::succeed;
            base.As(&swap_chain) >> must::succeed;
        }

        // Update rect and viewport
        {
            auto desc = DXGI_SWAP_CHAIN_DESC1{};
            swap_chain->GetDesc1(&desc) >> must::succeed;

            auto width = desc.Width;
            auto height = desc.Height;
            viewport.Width = static_cast<float>(width);
            viewport.Height = static_cast<float>(height);
            scissor_rect.right = static_cast<LONG>(width);
            scissor_rect.bottom = static_cast<LONG>(height);
        }

        // Create descriptor heap for render target view
        {
            auto desc = D3D12_DESCRIPTOR_HEAP_DESC{};
            desc.NumDescriptors = static_cast<UINT>(render_targets.size());
            desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
            desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
            device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&render_target_view_heap)) >> must::succeed;
        }

        // Create frame resources
        {
            auto offset = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
            auto handle = render_target_view_heap->GetCPUDescriptorHandleForHeapStart();

            // Create RTV for frames.
            for (auto index = UINT{}; auto & target : render_targets)
            {
                swap_chain->GetBuffer(index, IID_PPV_ARGS(&target));
                device->CreateRenderTargetView(target.Get(), nullptr, handle);

                handle.ptr += offset;
                index += 1;
            }
        }

        /////////////////////////////////////////////////////////////////////
        /// Setup resources

        // Create a root signature
        {
            auto feature = D3D12_FEATURE_DATA_ROOT_SIGNATURE{};
            feature.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1;

            auto range = std::array<D3D12_DESCRIPTOR_RANGE1, 1>{};
            range[0].BaseShaderRegister = 0;
            range[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
            range[0].NumDescriptors = 1;
            range[0].RegisterSpace = 0;
            range[0].OffsetInDescriptorsFromTableStart = 0;
            range[0].Flags = D3D12_DESCRIPTOR_RANGE_FLAG_NONE;

            auto parameter = std::array<D3D12_ROOT_PARAMETER1, 1>{};
            parameter[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
            parameter[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
            parameter[0].DescriptorTable.NumDescriptorRanges = static_cast<UINT>(range.size());
            parameter[0].DescriptorTable.pDescriptorRanges = range.data();

            auto desc = D3D12_VERSIONED_ROOT_SIGNATURE_DESC{};
            desc.Version = D3D_ROOT_SIGNATURE_VERSION_1_1; // tagged union of 1_0 and 1_1
            desc.Desc_1_1.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
            desc.Desc_1_1.NumParameters = static_cast<UINT>(parameter.size());
            desc.Desc_1_1.pParameters = parameter.data();
            desc.Desc_1_1.NumStaticSamplers = 0;
            desc.Desc_1_1.pStaticSamplers = nullptr;

            auto signature = ComPtr<ID3DBlob>();
            auto error = ComPtr<ID3DBlob>();
            D3D12SerializeVersionedRootSignature(&desc, &signature, &error) >> must::succeed_with(*error.Get());
            device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&root_signature)) >> must::succeed;
        }

        // Create pipeline state
        {
            auto input_vertex_layout = std::array<D3D12_INPUT_ELEMENT_DESC, 2>
            {
                D3D12_INPUT_ELEMENT_DESC
                {"POS", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
                {"TEX", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
            };

            auto vertex_shader = D3D12_SHADER_BYTECODE{};
            vertex_shader.pShaderBytecode = compiled_shader::vertex_shader;
            vertex_shader.BytecodeLength = sizeof(compiled_shader::vertex_shader);

            auto pixel_shader = D3D12_SHADER_BYTECODE{};
            pixel_shader.pShaderBytecode = compiled_shader::pixel_shader;
            pixel_shader.BytecodeLength = sizeof(compiled_shader::pixel_shader);

            auto render_target_blend = D3D12_RENDER_TARGET_BLEND_DESC{};
            render_target_blend.BlendEnable = false;
            render_target_blend.LogicOpEnable = false;
            render_target_blend.SrcBlend = D3D12_BLEND_ONE;
            render_target_blend.DestBlend = D3D12_BLEND_ZERO;
            render_target_blend.BlendOp = D3D12_BLEND_OP_ADD;
            render_target_blend.SrcBlendAlpha = D3D12_BLEND_ONE;
            render_target_blend.DestBlendAlpha = D3D12_BLEND_ZERO;
            render_target_blend.BlendOpAlpha = D3D12_BLEND_OP_ADD;
            render_target_blend.LogicOp = D3D12_LOGIC_OP_NOOP;
            render_target_blend.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

            auto blend = D3D12_BLEND_DESC{};
            blend.AlphaToCoverageEnable = false;
            blend.IndependentBlendEnable = false;
            for (auto & target : blend.RenderTarget)
                target = render_target_blend;

            auto raster = D3D12_RASTERIZER_DESC{};
            raster.FillMode = D3D12_FILL_MODE_SOLID;
            raster.CullMode = D3D12_CULL_MODE_NONE;
            raster.FrontCounterClockwise = false;
            raster.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
            raster.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
            raster.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
            raster.DepthClipEnable = true;
            raster.MultisampleEnable = false;
            raster.AntialiasedLineEnable = false;
            raster.ForcedSampleCount = 0;
            raster.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

            auto input_layout = D3D12_INPUT_LAYOUT_DESC{};
            input_layout.pInputElementDescs = input_vertex_layout.data();
            input_layout.NumElements = static_cast<UINT>(input_vertex_layout.size());

            // About graphics pipeline state:
            // https://learn.microsoft.com/en-us/windows/win32/direct3d12/managing-graphics-pipeline-state-in-direct3d-12#graphics-pipeline-states-set-with-pipeline-state-objects
            auto desc = D3D12_GRAPHICS_PIPELINE_STATE_DESC{};
            desc.pRootSignature = root_signature.Get();
            desc.VS = vertex_shader;
            desc.PS = pixel_shader;
            desc.BlendState = blend;
            desc.SampleMask = ~UINT{};
            desc.RasterizerState = raster;
            desc.DepthStencilState.DepthEnable = false;
            desc.DepthStencilState.StencilEnable = false;
            desc.InputLayout = input_layout;
            desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
            desc.NumRenderTargets = 1;
            desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
            desc.SampleDesc.Count = 1;

            device->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(&pipeline_state)) >> must::succeed;
        }

        // Create a closed command list
        {
            device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, command_allocator.Get(), pipeline_state.Get(), IID_PPV_ARGS(&command_list)) >> must::succeed;
            command_list->Close() >> must::succeed;

            // If we have "ID3D12Device4", use "CreateCommandList1" to aoivd calling "Close".
            // https://learn.microsoft.com/en-us/windows/win32/direct3d12/recording-command-lists-and-bundles#creating-command-lists
        }

        // Create a vertex buffer
        {

        }

        // TODO:

        /////////////////////////////////////////////////////////////////////
        /// Synchronization

        frame_index = swap_chain->GetCurrentBackBufferIndex();
        device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)) >> must::succeed;
    }

    // Environment
    wrl::ComPtr<ID3D12Device> device{};
    wrl::ComPtr<ID3D12DebugDevice> debug_device{};
    wrl::ComPtr<ID3D12CommandQueue> command_queue{};
    wrl::ComPtr<ID3D12CommandAllocator> command_allocator{};

    // Render target
    D3D12_VIEWPORT viewport{};
    D3D12_RECT scissor_rect{};
    wrl::ComPtr<IDXGISwapChain3> swap_chain{};
    wrl::ComPtr<ID3D12DescriptorHeap> render_target_view_heap{};
    std::array<wrl::ComPtr<ID3D12Resource>, 2> render_targets{};

    // Resources
    wrl::ComPtr<ID3D12RootSignature> root_signature{};
    wrl::ComPtr<ID3D12PipelineState> pipeline_state{};
    wrl::ComPtr<ID3D12GraphicsCommandList> command_list{};

    wrl::ComPtr<ID3D12Resource> index_buffer{};
    wrl::ComPtr<ID3D12Resource> vertex_buffer{};
    wrl::ComPtr<ID3D12Resource> uniform_buffer{};

    // Synchronization objects
    UINT frame_index{};
    HANDLE fence_event{};
    UINT64 fence_value{};
    wrl::ComPtr<ID3D12Fence> fence{};
};

// Tutorial used:
// https://learn.microsoft.com/en-us/windows/win32/direct3d12/design-philosophy-of-command-queues-and-command-lists
// https://learn.microsoft.com/en-us/windows/win32/direct3d12/creating-a-basic-direct3d-12-component
// https://learn.microsoft.com/en-us/samples/microsoft/directx-graphics-samples/d3d12-hello-world-samples-win32/
// https://alain.xyz/blog/raw-directx12

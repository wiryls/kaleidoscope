#include <concepts>
#include <stdexcept>
#include <system_error>
#include <array>

#define NOMINMAX
#include <Windows.h>
#include <D3d12SDKLayers.h>
#include <d3d12.h>
#include <d3d11.h>
#include <dxgi1_6.h>
#include <dcomp.h>
#include <wrl.h>

#include "tool.h"
#include "error.h"
#include "render.h"

// Run "build" to generate header files listed in compiled_shader.
//
// Refer to
// https://learn.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl-part1
//
namespace compiled_shader
{
#include "pixel_shader.h"
#include "vertex_shader.h"
}

// Rename namespace
namespace wrl = Microsoft::WRL;

// Builder
namespace make
{
    using wrl::ComPtr;

    auto static recommended_adapter(ComPtr<IDXGIFactory4> const & factory, ComPtr<IDXGIAdapter1> & adapter) -> HRESULT
    {
        auto current = ComPtr<IDXGIAdapter1>();
        auto desc = DXGI_ADAPTER_DESC1{};
        for (auto index = UINT{}; ; ++index)
        {
            // Note for ComPtr: "operator &" == "ReleaseAndGetAddressOf()"
            if (auto hr = factory->EnumAdapters1(index, &current); FAILED(hr))
                return hr;

            if (auto hr = current->GetDesc1(&desc); FAILED(hr))
                return hr;

            if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
                continue;

            adapter = std::move(current);

            // Common HRESULT Values
            // https://learn.microsoft.com/en-us/windows/win32/seccrypto/common-hresult-values
            return S_OK;
        }
    }

    auto static inferenced_output(HWND window, ComPtr<IDXGIAdapter1> const & adapter, ComPtr<IDXGIOutput1> & output) -> HRESULT
    {
        auto target = ::MonitorFromWindow(window, MONITOR_DEFAULTTONEAREST);
        auto source = ComPtr<IDXGIOutput>{};
        auto desc = DXGI_OUTPUT_DESC{};
        for (auto index = UINT{}; ; ++index)
        {
            if (auto hr = adapter->EnumOutputs(index, &source); FAILED(hr))
                return hr;

            if (auto hr = source->GetDesc(&desc); FAILED(hr))
                return hr;

            if (desc.Monitor == target)
                return source.As(&output);
        }
    }

    auto static viewport_and_scissor_rect(
        ComPtr<IDXGISwapChain3> const & swap_chain,
        D3D12_VIEWPORT & viewport,
        D3D12_RECT & scissor_rect) -> HRESULT
    {
        auto desc = DXGI_SWAP_CHAIN_DESC1{};
        auto hr = swap_chain->GetDesc1(&desc);
        if (FAILED(hr))
            return hr;

        auto width = std::max(desc.Width, UINT(1));
        auto height = std::max(desc.Height, UINT(1));
        viewport.Width = static_cast<float>(width);
        viewport.Height = static_cast<float>(height);
        scissor_rect.right = static_cast<LONG>(width);
        scissor_rect.bottom = static_cast<LONG>(height);

        return S_OK;
    }

    auto static render_target_views(
        ComPtr<ID3D12Device> const & device,
        ComPtr<IDXGISwapChain3> const & swap_chain,
        ComPtr<ID3D12DescriptorHeap> & render_target_view_descriptor_heap, /* out */
        std::array<ComPtr<ID3D12Resource>, 2> & render_targets  /* out */) -> HRESULT
    {
        // Refer to:
        // https://learn.microsoft.com/en-us/windows/win32/direct3d12/creating-descriptor-heaps

        // Create a descriptor heap for render target views
        auto desc = D3D12_DESCRIPTOR_HEAP_DESC{};
        desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        desc.NumDescriptors = static_cast<UINT>(render_targets.size());
        desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        auto hr = device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&render_target_view_descriptor_heap));
        if (FAILED(hr))
            return hr;

        // Create two render target views for swap chain
        auto offset = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        auto handle = render_target_view_descriptor_heap->GetCPUDescriptorHandleForHeapStart();
        for (auto index = UINT{}; auto & target : render_targets)
        {
            hr = swap_chain->GetBuffer(index, IID_PPV_ARGS(&target));
            if (FAILED(hr))
                return hr;

            device->CreateRenderTargetView(target.Get(), nullptr, handle);

            handle.ptr += offset;
            index += 1;
        }

        // Done
        return S_OK;
    }

    // Create a D3D11 texture and share it to D3D12 device
    auto static shared_texture2d(
        ComPtr<ID3D11Device> const & device11,
        ComPtr<ID3D12Device> const & device12,
        ComPtr<ID3D12DescriptorHeap> const & heap,
        UINT heap_handle_offset,
        ComPtr<ID3D11Texture2D> & texture,
        ComPtr<ID3D12Resource> & resource,
        HANDLE & shared_handle,
        UINT width,
        UINT height) -> HRESULT
    {
        // Create a texture for screenshot
        //
        // Format must be DXGI_FORMAT_B8G8R8A8_UNORM
        // MiscFlags must be D3D11_RESOURCE_MISC_SHARED_NTHANDLE
        auto desc = D3D11_TEXTURE2D_DESC{};
        desc.Width = width;
        desc.Height = height;
        desc.MipLevels = 1;
        desc.ArraySize = 1;
        desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
        desc.SampleDesc = {1, 0};
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        desc.MiscFlags = D3D11_RESOURCE_MISC_SHARED | D3D11_RESOURCE_MISC_SHARED_NTHANDLE;
        auto hr = device11->CreateTexture2D(&desc, nullptr, &texture);

        // Create a shared handle
        auto shared_resource = ComPtr<IDXGIResource1>{};
        if (SUCCEEDED(hr))
            hr = texture.As(&shared_resource);

        if (SUCCEEDED(hr))
        {
            if (shared_handle != nullptr)
            {
                CloseHandle(shared_handle);
                shared_handle = nullptr;
            }
            hr = shared_resource->CreateSharedHandle(nullptr, GENERIC_ALL, nullptr, &shared_handle);
        }

        if (SUCCEEDED(hr))
        {
            hr = device12->OpenSharedHandle(shared_handle, IID_PPV_ARGS(&resource));
        }

        if (SUCCEEDED(hr))
        {
            auto view_desc = D3D12_SHADER_RESOURCE_VIEW_DESC{};
            view_desc.Format = desc.Format;
            view_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
            view_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING; // no extra mapping
            view_desc.Texture2D.MostDetailedMip = 0; // most detailed mipmap level to use
            view_desc.Texture2D.MipLevels = desc.MipLevels;
            view_desc.Texture2D.ResourceMinLODClamp = 0.f; // minimum mipmap level

            auto offset = heap->GetCPUDescriptorHandleForHeapStart();
            offset.ptr += heap_handle_offset;
            device12->CreateShaderResourceView(resource.Get(), &view_desc, offset);
        }

        return hr;
    }
}

// Details
struct mirror::core
{
public:

    core(HWND window, UINT width, UINT height)
        : window_instance(window)
        , window_width(width)
        , window_height(height)
    {
        // Prepare
        using namespace aux;
        using wrl::ComPtr;

        // Expected feature levels
        auto constexpr feature_levels = std::array<D3D_FEATURE_LEVEL, 1>
        {
            D3D_FEATURE_LEVEL_11_0
        };

        /////////////////////////////////////////////////////////////////////
        /// Device and Command Queue

        // Create a debug layer.
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

        // Create a DXGI factory and adapter
        auto factory = ComPtr<IDXGIFactory4>();
        auto adapter = ComPtr<IDXGIAdapter1>{};
        {
            auto flag = env::is_debug() ? DXGI_CREATE_FACTORY_DEBUG : UINT{};
            CreateDXGIFactory2(flag, IID_PPV_ARGS(&factory)) >> must::succeed;
            make::recommended_adapter(factory, adapter) >> must::succeed;
        }

        // Create device, command queue and command allocator
        {
            // Device
            D3D12CreateDevice(adapter.Get(), feature_levels.back(), IID_PPV_ARGS(&device)) >> must::succeed;
            if constexpr (env::is_debug())
                device.As(&debug_device) >> must::succeed;

            // Command queue
            auto desc = D3D12_COMMAND_QUEUE_DESC{};
            desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
            desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
            device->CreateCommandQueue(&desc, IID_PPV_ARGS(&command_queue)) >> must::succeed;

            // Command allocator
            device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&command_allocator)) >> must::succeed;
        }

        /////////////////////////////////////////////////////////////////////
        /// Swap chain

        // Create swap chain
        {
            // Note: zero width and height means using client area of the target window
            //
            // Refer to
            // https://learn.microsoft.com/en-us/windows/win32/api/dxgi1_2/ns-dxgi1_2-dxgi_swap_chain_desc1
            auto desc = DXGI_SWAP_CHAIN_DESC1{};
            desc.Width = width;
            desc.Height = height;
            desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            desc.SampleDesc.Count = 1;
            desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
            desc.BufferCount = static_cast<UINT>(render_targets.size());
            desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
            desc.AlphaMode = DXGI_ALPHA_MODE_PREMULTIPLIED;

            // Must use command_queue (in DX12) instead of device (in DX11)
            //
            // Refer to
            // https://learn.microsoft.com/en-us/windows/win32/api/dxgi1_2/nf-dxgi1_2-idxgifactory2-createswapchainforcomposition
            auto base = ComPtr<IDXGISwapChain1>();
            factory->CreateSwapChainForComposition(command_queue.Get(), &desc, nullptr, &base) >> must::succeed;
            base.As(&swap_chain) >> must::succeed;
        }

        // Create swap chain related objects:
        //
        // - Update rect and viewport
        // - Create a descriptor heap and its render target views,
        // - Update descriptor size and frame index
        {
            make::viewport_and_scissor_rect(swap_chain, viewport, scissor_rect) >> must::succeed;
            make::render_target_views(device, swap_chain, render_target_view_descriptor_heap, render_targets) >> must::succeed;
            render_target_view_descriptor_size = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
            back_buffer_index = swap_chain->GetCurrentBackBufferIndex();
        }

        /////////////////////////////////////////////////////////////////////
        /// Create composition device
        {
            // Refer to:
            // https://github.com/PJayB/DirectCompositionDirectX12Sample/blob/689c151d0358bc7745ca6e4a3c4f35bbfa75941b/DirectCompositeSample.cpp#L107-L125
            DCompositionCreateDevice(nullptr, IID_PPV_ARGS(&composition_device)) >> must::succeed;
            composition_device->CreateTargetForHwnd(window, true, &composition_target) >> must::succeed;
            composition_device->CreateVisual(&composition_visual) >> must::succeed;
            composition_visual->SetContent(swap_chain.Get()) >> must::succeed;
            composition_target->SetRoot(composition_visual.Get()) >> must::succeed;
            composition_device->Commit() >> must::succeed;
        }

        /////////////////////////////////////////////////////////////////////
        /// Create an IDXGIOutput
        {
            // Device and context
            D3D11CreateDevice(
                adapter.Get(),
                D3D_DRIVER_TYPE_UNKNOWN,
                nullptr,
                0,
                feature_levels.data(),
                static_cast<UINT>(feature_levels.size()),
                D3D11_SDK_VERSION,
                &device11,
                nullptr,
                &context11) >> must::succeed;

            // Create an IDXGIOutput1
            make::inferenced_output(window, adapter, output) >> must::succeed;

            // Create an IDXGIDuplicateOutput
            output->DuplicateOutput(device11.Get(), &output_duplication) >> must::succeed;
        }

        /////////////////////////////////////////////////////////////////////
        /// Setup resources

        // Create a root signature
        //
        // "one for every different binding configuration an application needs."
        // Refer to: https://learn.microsoft.com/en-us/windows/win32/direct3d12/resource-binding-flow-of-control#resource-binding-flow-of-control
        {
            // Prepare a static sampler for pixel shader
            //
            // Refer to
            // https://www.3dgep.com/learning-directx-12-4/#Texture_Sampler
            // https://github.com/microsoft/DirectX-Graphics-Samples/blob/a79e01c4c39e6d40f4b078688ff95814d166d34f/Samples/Desktop/D3D12HelloWorld/src/HelloTexture/D3D12HelloTexture.cpp#L165-L178
            auto samplers = std::array<D3D12_STATIC_SAMPLER_DESC, 1>{};
            samplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
            samplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
            samplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
            samplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
            samplers[0].MipLODBias = 0;
            samplers[0].MaxAnisotropy = 0;
            samplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
            samplers[0].BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
            samplers[0].MinLOD = 0.0f;
            samplers[0].MaxLOD = D3D12_FLOAT32_MAX;
            samplers[0].ShaderRegister = 0;
            samplers[0].RegisterSpace = 0;
            samplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

            // Prepare ranges to maintain descriptors of CBV, SRV and UAV
            auto ranges = std::array<D3D12_DESCRIPTOR_RANGE1, 2>{};
            {
                // constant buffer
                ranges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV; // Constant buffer view
                ranges[0].NumDescriptors = 1;                          // There will be one descriptor
                ranges[0].BaseShaderRegister = 0;                      // Map to "register(t0)"
                ranges[0].RegisterSpace = 0;                           // Map to "register(_, space0)"
                ranges[0].Flags = D3D12_DESCRIPTOR_RANGE_FLAG_NONE;
                ranges[0].OffsetInDescriptorsFromTableStart = 0;

                // Prepare for shader resource view (texture)
                ranges[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
                ranges[1].NumDescriptors = 1;
                ranges[1].BaseShaderRegister = 0;
                ranges[1].RegisterSpace = 0;
                ranges[1].Flags = D3D12_DESCRIPTOR_RANGE_FLAG_NONE;
                ranges[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
            }

            // Create parameters
            auto parameters = std::array<D3D12_ROOT_PARAMETER1, 1>{};
            parameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
            parameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
            parameters[0].DescriptorTable.NumDescriptorRanges = static_cast<UINT>(ranges.size());
            parameters[0].DescriptorTable.pDescriptorRanges = ranges.data();

            // Allow input layout and deny uneccessary access to certain pipeline stages.
            //
            // Refer to
            // https://github.com/microsoft/DirectX-Graphics-Samples/blob/a79e01c4c39e6d40f4b078688ff95814d166d34f/Samples/Desktop/D3D12HelloWorld/src/HelloConstBuffers/D3D12HelloConstBuffers.cpp#L170
            auto flag
                = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
                | D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS
                | D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS
                | D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

            // We could use "device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, ...)" to check if 1_1 is supported
            auto desc = D3D12_VERSIONED_ROOT_SIGNATURE_DESC{};
            desc.Version = D3D_ROOT_SIGNATURE_VERSION_1_1; // tagged union of 1_0 and 1_1
            desc.Desc_1_1.Flags = flag;
            desc.Desc_1_1.NumParameters = static_cast<UINT>(parameters.size());
            desc.Desc_1_1.pParameters = parameters.data();
            desc.Desc_1_1.NumStaticSamplers = static_cast<UINT>(samplers.size());
            desc.Desc_1_1.pStaticSamplers = samplers.data();

            auto signature = ComPtr<ID3DBlob>();
            auto error = ComPtr<ID3DBlob>();
            D3D12SerializeVersionedRootSignature(&desc, &signature, &error) >> must::succeed_with(*error.Get());
            device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&root_signature)) >> must::succeed;
        }

        // Create a pipeline state (with vertex shader and pixel shader)
        {
            // Create the pipeline
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

            auto input_layout_desc = D3D12_INPUT_LAYOUT_DESC{};
            input_layout_desc.pInputElementDescs = input_layout.data();
            input_layout_desc.NumElements = static_cast<UINT>(input_layout.size());

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
            desc.InputLayout = input_layout_desc;
            desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
            desc.NumRenderTargets = 1;
            desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
            desc.SampleDesc.Count = 1;

            device->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(&pipeline_state)) >> must::succeed;
        }

        // Create a command list
        {
            device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, command_allocator.Get(), pipeline_state.Get(), IID_PPV_ARGS(&command_list)) >> must::succeed;
            command_list->Close() >> must::succeed;

            // Note: by default, command list is ceated in recording state.
            //
            // If "ID3D12Device4" is available, use "CreateCommandList1" instead.
            // https://learn.microsoft.com/en-us/windows/win32/direct3d12/recording-command-lists-and-bundles#creating-command-lists
        }

        // Create a constant buffer (to track triangle's top and side lengh)
        //
        // Also see
        // https://github.com/microsoft/DirectX-Graphics-Samples/blob/a79e01c4c39e6d40f4b078688ff95814d166d34f/Samples/Desktop/D3D12HelloWorld/src/HelloConstBuffers/D3D12HelloConstBuffers.cpp
        {
            auto constexpr size = sizeof(triangle_constant_buffer);
            auto constexpr aligned_size = (size + 0xff) & ~0xff; // Required to be 256-byte (D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT) aligned

            // Create a descriptor heap for our constant buffer and texture
            auto heap_desc = D3D12_DESCRIPTOR_HEAP_DESC{};
            heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
            heap_desc.NumDescriptors = 2; // constant buffer + shader resource
            heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
            device->CreateDescriptorHeap(&heap_desc, IID_PPV_ARGS(&descriptor_heap)) >> must::succeed;

            auto heap_properties = D3D12_HEAP_PROPERTIES{};
            heap_properties.Type = D3D12_HEAP_TYPE_UPLOAD;
            heap_properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
            heap_properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
            heap_properties.CreationNodeMask = 1;
            heap_properties.VisibleNodeMask = 1;

            // Create a constant buffer
            //
            // Refer to
            // https://learn.microsoft.com/en-us/windows/win32/api/d3d12/ns-d3d12-d3d12_resource_desc#buffers
            auto resource_desc = D3D12_RESOURCE_DESC{};
            resource_desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
            resource_desc.Alignment = 0; // Same as D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT
            resource_desc.Width = aligned_size;
            resource_desc.Height = 1;
            resource_desc.DepthOrArraySize = 1;
            resource_desc.MipLevels = 1;
            resource_desc.Format = DXGI_FORMAT_UNKNOWN;
            resource_desc.SampleDesc.Count = 1;
            resource_desc.SampleDesc.Quality = 0;
            resource_desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
            resource_desc.Flags = D3D12_RESOURCE_FLAG_NONE;

            device->CreateCommittedResource(
                &heap_properties,
                D3D12_HEAP_FLAG_NONE,
                &resource_desc,
                D3D12_RESOURCE_STATE_GENERIC_READ,
                nullptr,
                IID_PPV_ARGS(&constant_buffer)) >> must::succeed;

            // Create a constant buffer view
            auto constant_buffer_view_desc = D3D12_CONSTANT_BUFFER_VIEW_DESC{};
            constant_buffer_view_desc.BufferLocation = constant_buffer->GetGPUVirtualAddress();
            constant_buffer_view_desc.SizeInBytes = aligned_size;
            auto constant_buffer_handle = descriptor_heap->GetCPUDescriptorHandleForHeapStart();
            device->CreateConstantBufferView(&constant_buffer_view_desc, constant_buffer_handle);

            // Note:
            //
            // - Empty range means CPU won't read the data
            // - Unmap is not needed for constant buffer
            //
            // Refer to
            // https://learn.microsoft.com/en-us/windows/win32/api/d3d12/nf-d3d12-id3d12resource-map
            auto range = D3D12_RANGE{};
            constant_buffer->Map(0, &range, reinterpret_cast<void **>(&constant_buffer_data)) >> must::succeed;

            // Update offsets
            descriptor_heap_offsets[0] = 0;
            descriptor_heap_offsets[1] = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        }

        // Create a vertex buffer (to fill the entire screen)
        {
            // Create the vertex buffer
            auto heap_properties = D3D12_HEAP_PROPERTIES{};
            heap_properties.Type = D3D12_HEAP_TYPE_UPLOAD;
            heap_properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
            heap_properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
            heap_properties.CreationNodeMask = 1;
            heap_properties.VisibleNodeMask = 1;

            auto resource_desc = D3D12_RESOURCE_DESC{};
            resource_desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
            resource_desc.Alignment = 0;
            resource_desc.Width = sizeof(vertices);
            resource_desc.Height = 1;
            resource_desc.DepthOrArraySize = 1;
            resource_desc.MipLevels = 1;
            resource_desc.Format = DXGI_FORMAT_UNKNOWN;
            resource_desc.SampleDesc.Count = 1;
            resource_desc.SampleDesc.Quality = 0;
            resource_desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
            resource_desc.Flags = D3D12_RESOURCE_FLAG_NONE;

            device->CreateCommittedResource(
                &heap_properties,
                D3D12_HEAP_FLAG_NONE,
                &resource_desc,
                D3D12_RESOURCE_STATE_GENERIC_READ,
                nullptr,
                IID_PPV_ARGS(&vertex_buffer)) >> must::succeed;

            // Copy vertices to vertex buffer
            auto input = static_cast<UINT8 *>(nullptr);
            auto range = D3D12_RANGE{};
            vertex_buffer->Map(0, &range, reinterpret_cast<void **>(&input)) >> must::succeed;
            std::memcpy(input, vertices.data(), sizeof(vertices));
            vertex_buffer->Unmap(0, nullptr);

            // Initialize the vertex buffer view
            vertex_buffer_view.BufferLocation = vertex_buffer->GetGPUVirtualAddress();
            vertex_buffer_view.StrideInBytes = sizeof(vertices.front());
            vertex_buffer_view.SizeInBytes = sizeof(vertices);
        }

        // Create a index buffer
        {
            // Create 
            auto heap_properties = D3D12_HEAP_PROPERTIES{};
            heap_properties.Type = D3D12_HEAP_TYPE_UPLOAD;
            heap_properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
            heap_properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
            heap_properties.CreationNodeMask = 1;
            heap_properties.VisibleNodeMask = 1;

            auto resource_desc = D3D12_RESOURCE_DESC{};
            resource_desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
            resource_desc.Alignment = 0;
            resource_desc.Width = sizeof(indexes);
            resource_desc.Height = 1;
            resource_desc.DepthOrArraySize = 1;
            resource_desc.MipLevels = 1;
            resource_desc.Format = DXGI_FORMAT_UNKNOWN;
            resource_desc.SampleDesc.Count = 1;
            resource_desc.SampleDesc.Quality = 0;
            resource_desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
            resource_desc.Flags = D3D12_RESOURCE_FLAG_NONE;

            device->CreateCommittedResource(
                &heap_properties,
                D3D12_HEAP_FLAG_NONE,
                &resource_desc,
                D3D12_RESOURCE_STATE_GENERIC_READ,
                nullptr,
                IID_PPV_ARGS(&index_buffer)) >> must::succeed;

            // Copy indexes array to index buffer
            auto input = static_cast<UINT8 *>(nullptr);
            auto range = D3D12_RANGE{};
            index_buffer->Map(0, &range, reinterpret_cast<void **>(&input)) >> must::succeed;
            std::memcpy(input, indexes.data(), sizeof(indexes));
            index_buffer->Unmap(0, nullptr);

            // Initialize the vertex buffer view
            index_buffer_view.BufferLocation = index_buffer->GetGPUVirtualAddress();
            index_buffer_view.Format = DXGI_FORMAT_R32_UINT;
            index_buffer_view.SizeInBytes = sizeof(indexes);
        }

        // Create a texture for resource view
        {
            make::shared_texture2d(
                device11,
                device,
                descriptor_heap,
                descriptor_heap_offsets[1],
                shared_texture,
                screenshot_texture,
                shared_texture_handle,
                width,
                height) >> must::succeed;
        }

        /////////////////////////////////////////////////////////////////////
        /// Synchronization objects

        // Create Synchronization objects
        {
            fence_event = CreateEvent(nullptr, false, false, nullptr) >> must::non_null;
            fence_value = 1;
            device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)) >> must::succeed;
        }
    }

    ~core()
    {
        wait_for_previous_frame();
        if (shared_texture_handle != nullptr)
            CloseHandle(shared_texture_handle);
        if (fence_event != nullptr)
            CloseHandle(fence_event);
    }

    auto wait_for_previous_frame() -> void
    {
        using namespace aux;

        auto value = fence_value;
        command_queue->Signal(fence.Get(), value) >> must::succeed;

        if (++fence_value; fence->GetCompletedValue() < value)
        {
            fence->SetEventOnCompletion(value, fence_event) >> must::succeed;
            WaitForSingleObjectEx(fence_event, INFINITE, false);
        }

        // Refer to:
        // https://github.com/microsoft/DirectX-Graphics-Samples/blob/0aa79bad78992da0b6a8279ddb9002c1753cb849/Samples/Desktop/D3D12HelloWorld/src/HelloTriangle/D3D12HelloTriangle.cpp#L320-L340
    }

    auto update_screenshot() -> void
    {
        using namespace aux;
        using wrl::ComPtr;

        // Note:
        //
        // > For performance reasons, we recommend that you release the frame
        // > just before you call the IDXGIOutputDuplication::AcquireNextFrame
        // > method to acquire the next frame.
        //
        // Refer to
        // https://learn.microsoft.com/en-us/windows/win32/api/dxgi1_2/nf-dxgi1_2-idxgioutputduplication-releaseframe
        
        switch (auto hr = output_duplication->ReleaseFrame(); hr)
        {
        case DXGI_ERROR_ACCESS_LOST:
            // Need to create a new DuplicateOutput
            //
            // Refer to
            // https://learn.microsoft.com/en-us/windows/win32/api/dxgi1_2/nn-dxgi1_2-idxgioutputduplication
            // https://stackoverflow.com/a/31238973
            output->DuplicateOutput(device11.Get(), &output_duplication) >> must::succeed;
            break;

        case DXGI_ERROR_INVALID_CALL:
            // Already released, so ignore it
            break;

        default:
            hr >> must::succeed;
            break;
        }

        auto hr = S_OK;
        auto frame_info = DXGI_OUTDUPL_FRAME_INFO{};
        auto frame_resource = ComPtr<IDXGIResource>{};

        if (hr = output_duplication->AcquireNextFrame(0, &frame_info, &frame_resource); hr == DXGI_ERROR_ACCESS_LOST)
        {
            // Retry AcquireNextFrame once
            output->DuplicateOutput(device11.Get(), &output_duplication) >> must::succeed;
            hr = output_duplication->AcquireNextFrame(0, &frame_info, &frame_resource);
        }

        if (FAILED(hr) && hr != DXGI_ERROR_WAIT_TIMEOUT)
        {
            hr >> must::succeed;
        }
        else if (hr == DXGI_ERROR_WAIT_TIMEOUT || frame_info.LastPresentTime.QuadPart == 0)
        {
            // Zero LastPresentTime means there is nothing changed, no need to update.
            //
            // Refer to
            // https://learn.microsoft.com/en-us/windows/win32/api/dxgi1_2/ns-dxgi1_2-dxgi_outdupl_frame_info#members
            return;
        }

        auto screenshot = ComPtr<ID3D11Texture2D>{};
        frame_resource.As(&screenshot) >> must::succeed;

        // Maybe resize
        auto source = D3D11_TEXTURE2D_DESC{};
        screenshot->GetDesc(&source);
        auto target = D3D11_TEXTURE2D_DESC{};
        shared_texture->GetDesc(&target);
        if (source.Width != target.Width || source.Height != target.Height)
        {
            make::shared_texture2d(
                device11,
                device,
                descriptor_heap,
                descriptor_heap_offsets[1],
                shared_texture,
                screenshot_texture,
                shared_texture_handle,
                source.Width,
                source.Height) >> must::succeed;
        }

        // Copy
        context11->CopyResource(shared_texture.Get(), screenshot.Get());
    }

    auto on_resize(UINT width, UINT height) -> void
    {
        using namespace aux;

        // 0. Update info
        window_width = width;
        window_height = height;

        // 1. Wait for command_queue finished
        wait_for_previous_frame();

        // 2. Release old references
        //
        // Refer to
        // https://learn.microsoft.com/en-us/windows/win32/api/dxgi/nf-dxgi-idxgiswapchain-resizebuffers#remarks
        for (auto & target : render_targets)
        {
            target.Reset();
        }
        render_target_view_descriptor_heap.Reset();

        // 3. Resize swapchain buffer
        swap_chain->ResizeBuffers(static_cast<UINT>(render_targets.size()), width, height, DXGI_FORMAT_R8G8B8A8_UNORM, 0) >> must::succeed;

        // 4. Resize viewport and rect
        make::viewport_and_scissor_rect(swap_chain, viewport, scissor_rect) >> must::succeed;

        // 5. Create a new DescriptorHeap for render target view, and two render_targets
        make::render_target_views(device, swap_chain, render_target_view_descriptor_heap, render_targets) >> must::succeed;
        render_target_view_descriptor_size = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        back_buffer_index = swap_chain->GetCurrentBackBufferIndex();

        // 6. Create a new shared screenshot texture
        make::shared_texture2d(
            device11,
            device,
            descriptor_heap,
            descriptor_heap_offsets[1],
            shared_texture,
            screenshot_texture,
            shared_texture_handle,
            width,
            height) >> must::succeed;
    }

    auto on_update(aligned_regular_triangle const & source) -> void
    {
        // Helper
        auto constexpr half_sqrt3 = 0.86602540378443864676372317075294f;
        auto w = static_cast<float>(window_width);
        auto h = static_cast<float>(window_height);

        // Update constant buffer (normalization)
        auto target = triangle_constant_buffer{};
        target.top_x = source.top_x / w;
        target.top_y = source.top_y / h;
        target.length = source.length / w;
        target.height = source.length * half_sqrt3 / h;

        std::memcpy(constant_buffer_data, &target, sizeof(target));
    }

    auto on_render() -> void
    {
        using namespace aux;
        using wrl::ComPtr;

        // Wait until done
        wait_for_previous_frame();
        back_buffer_index = swap_chain->GetCurrentBackBufferIndex();

        // Grab a screenshot from OutputDuplication
        update_screenshot();

        // Reset command list
        command_allocator->Reset() >> must::succeed;
        command_list->Reset(command_allocator.Get(), pipeline_state.Get()) >> must::succeed;

        // Setup command list
        command_list->SetGraphicsRootSignature(root_signature.Get());
        command_list->RSSetViewports(1, &viewport);
        command_list->RSSetScissorRects(1, &scissor_rect);

        // Setup descriptor heaps
        //
        // Note: we don't have any sampler descriptor heaps, so just use "descriptor_heap" as an
        // array.
        //
        // > Only one descriptor heap of each type can be set at one time, which means a maximum
        // > of 2 heaps (one sampler, one CBV/SRV/UAV) can be set at one time.
        //
        // Refer to
        // https://learn.microsoft.com/en-us/windows/win32/api/d3d12/nf-d3d12-id3d12graphicscommandlist-setdescriptorheaps
        // https://stackoverflow.com/a/32145373
        command_list->SetDescriptorHeaps(1, descriptor_heap.GetAddressOf());
        command_list->SetGraphicsRootDescriptorTable(0, descriptor_heap->GetGPUDescriptorHandleForHeapStart());

        // Switch to STATE_RENDER_TARGET and fill the back buffer
        //
        // Refer to
        // https://github.com/microsoft/DirectXTK12/wiki/Resource-Barriers#swap-chain-resource-states
        auto render_target_barrier = D3D12_RESOURCE_BARRIER{};
        render_target_barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        render_target_barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        render_target_barrier.Transition.pResource = render_targets[back_buffer_index].Get(); // Use back buffer
        render_target_barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
        render_target_barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
        render_target_barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        command_list->ResourceBarrier(1, &render_target_barrier);

        auto handle = render_target_view_descriptor_heap->GetCPUDescriptorHandleForHeapStart();
        handle.ptr += static_cast<SIZE_T>(back_buffer_index * render_target_view_descriptor_size);
        command_list->OMSetRenderTargets(1, &handle, false, nullptr);

        // Clear and draw
        auto constexpr default_color = std::array<float, 4>{0.f, 0.f, 0.f, 0.f};
        command_list->ClearRenderTargetView(handle, default_color.data(), 0, nullptr);
        command_list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        command_list->IASetVertexBuffers(0, 1, &vertex_buffer_view);
        command_list->IASetIndexBuffer(&index_buffer_view);
        command_list->DrawIndexedInstanced(static_cast<UINT>(indexes.size()), 1, 0, 0, 0);

        // Switch to STATE_PRESENT and present the back buffer
        auto present_barrier = D3D12_RESOURCE_BARRIER{};
        present_barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        present_barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        present_barrier.Transition.pResource = render_targets[back_buffer_index].Get();
        present_barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
        present_barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
        present_barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        command_list->ResourceBarrier(1, &present_barrier);
        
        // Close and execute command_list
        auto command_lists = std::array<ID3D12CommandList *, 1>{ command_list.Get() };
        command_list->Close() >> must::succeed;
        command_queue->ExecuteCommandLists(static_cast<UINT>(command_lists.size()), command_lists.data());

        // Present back buffer
        swap_chain->Present(1, 0) >> must::succeed;
    }

public:
    // Define constant buffer layout
    struct triangle_constant_buffer
    {
        float top_x;
        float top_y;
        float length;
        float height;
    };

    // Define the input layout for vertex shader
    auto static inline constexpr input_layout = std::array<D3D12_INPUT_ELEMENT_DESC, 2>
    {
        D3D12_INPUT_ELEMENT_DESC
        { "POS", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEX", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };

    // 4 pairs of (pos.x, pos.y, tex.x, tex.y)
    auto static inline constexpr vertices = std::array<std::array<float, 4>, 4>
    {
        -1.f, +1.f, 0.f, 0.f, // top left
        +1.f, +1.f, 1.f, 0.f, // top right
        +1.f, -1.f, 1.f, 1.f, // bottom right
        -1.f, -1.f, 0.f, 1.f, // bottom left
    };

    // Divide one screen rect into two triangles
    auto static inline constexpr indexes = std::array<std::uint32_t, 3 * 2>
    {
        0, 1, 2, // top left -> top right -> bottom right
        0, 2, 3, // top left -> bottom right -> bottom left
    };

public:
    HWND window_instance;
    UINT window_width;
    UINT window_height;

    // Device and Command Queue
    wrl::ComPtr<ID3D12Device> device{};
    wrl::ComPtr<ID3D12DebugDevice> debug_device{};
    wrl::ComPtr<ID3D12CommandQueue> command_queue{};
    wrl::ComPtr<ID3D12CommandAllocator> command_allocator{};

    // Swap chain
    D3D12_VIEWPORT viewport{};
    D3D12_RECT scissor_rect{};
    wrl::ComPtr<IDXGISwapChain3> swap_chain{};
    wrl::ComPtr<ID3D12DescriptorHeap> render_target_view_descriptor_heap{};
    std::array<wrl::ComPtr<ID3D12Resource>, 2> render_targets{};
    UINT render_target_view_descriptor_size{};
    UINT back_buffer_index{};

    // Composition
    wrl::ComPtr<IDCompositionDevice> composition_device{};
    wrl::ComPtr<IDCompositionTarget> composition_target{};
    wrl::ComPtr<IDCompositionVisual> composition_visual{};

    // D3D11 Device and OutputDuplication
    //
    // As IDXGIOutputDuplication is not supported on D3D12, we need to maintain a
    // stand-alone D3D11 device. Hope to remove it once supported.
    //
    // Refer to
    // https://stackoverflow.com/a/40294831
    wrl::ComPtr<ID3D11Device> device11{};
    wrl::ComPtr<ID3D11DeviceContext> context11{};
    wrl::ComPtr<IDXGIOutput1> output{};
    wrl::ComPtr<IDXGIOutputDuplication> output_duplication{};

    // Resources
    wrl::ComPtr<ID3D12RootSignature> root_signature{};
    wrl::ComPtr<ID3D12PipelineState> pipeline_state{};
    wrl::ComPtr<ID3D12DescriptorHeap> descriptor_heap{};
    wrl::ComPtr<ID3D12GraphicsCommandList> command_list{};
    std::array<UINT, 2> descriptor_heap_offsets{};

    wrl::ComPtr<ID3D12Resource> constant_buffer{};
    UINT8 * constant_buffer_data{};

    wrl::ComPtr<ID3D12Resource> vertex_buffer{};
    D3D12_VERTEX_BUFFER_VIEW vertex_buffer_view{};

    wrl::ComPtr<ID3D12Resource> index_buffer{};
    D3D12_INDEX_BUFFER_VIEW index_buffer_view{};

    wrl::ComPtr<ID3D11Texture2D> shared_texture{};
    wrl::ComPtr<ID3D12Resource> screenshot_texture{};
    HANDLE shared_texture_handle{};

    // Synchronization objects
    HANDLE fence_event{};
    UINT64 fence_value{};
    wrl::ComPtr<ID3D12Fence> fence{};
};

// Thanks to:
// - https://alain.xyz/blog/raw-directx12
// - https://learn.microsoft.com/en-us/windows/win32/direct3d12/design-philosophy-of-command-queues-and-command-lists
// - https://learn.microsoft.com/en-us/windows/win32/direct3d12/creating-a-basic-direct3d-12-component
// - https://learn.microsoft.com/en-us/samples/microsoft/directx-graphics-samples/d3d12-hello-world-samples-win32/
// - https://learn.microsoft.com/en-us/archive/msdn-magazine/2014/june/windows-with-c-high-performance-window-layering-using-the-windows-composition-engine
// - https://logins.github.io/graphics/2020/07/31/DX12ResourceHandling.html

mirror::~mirror() = default;
mirror::mirror(HWND window, std::uint32_t width, std::uint32_t height)
    : o(std::make_unique<core>(window, static_cast<UINT>(width), static_cast<UINT>(height)))
{}

auto mirror::on_resize(std::uint32_t width, std::uint32_t height) -> void
{
    o->on_resize(static_cast<UINT>(width), static_cast<UINT>(height));
}

auto mirror::on_render() -> void
{
    o->on_render();
}

auto mirror::on_update(aligned_regular_triangle const & triangle) -> void
{
    o->on_update(triangle);
}

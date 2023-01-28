#define NOMINMAX
#include <Windows.h>
#include <D3d12SDKLayers.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
#include <comdef.h>

#include "tool.h"

// About the compiled shader headers:
// https://learn.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl-part1
namespace compiled_shader
{
#include "pixel_shader.h"
#include "vertex_shader.h"
}
namespace wrl = Microsoft::WRL;
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
        ::OutputDebugString(desc);

        throw err;
    };
}

struct core
{
    core()
    {
        using namespace aux;
        using wrl::ComPtr;

#if defined(_DEBUG)
        // debug layer
        {
            auto c = ComPtr<ID3D12Debug>();
            auto i = ComPtr<ID3D12Debug1>();
            D3D12GetDebugInterface(IID_PPV_ARGS(&c)) >> must::succeed;
            c->QueryInterface(i.GetAddressOf()) >> must::succeed;
            i->EnableDebugLayer();
            i->SetEnableGPUBasedValidation(true);
        }
#endif

        // DXGI factory
        auto factory = ComPtr<IDXGIFactory4>();
        CreateDXGIFactory1(IID_PPV_ARGS(&factory)) >> must::succeed;

        // TODO:
    }

    D3D12_VIEWPORT viewport;
    D3D12_RECT rect;
    wrl::ComPtr<IDXGISwapChain3> swap_chain;
    wrl::ComPtr<ID3D12Device> device;
    wrl::ComPtr<ID3D12Resource> render_targets;
    wrl::ComPtr<ID3D12CommandAllocator> command_allocator;
    wrl::ComPtr<ID3D12CommandQueue> command_queue;
    wrl::ComPtr<ID3D12RootSignature> root_signature;
    wrl::ComPtr<ID3D12DescriptorHeap> render_target_view_heap;
    wrl::ComPtr<ID3D12PipelineState> pipeline_state;
    wrl::ComPtr<ID3D12GraphicsCommandList> command_list;
};

// Tutorial used:
// https://learn.microsoft.com/en-us/windows/win32/direct3d12/creating-a-basic-direct3d-12-component
// https://learn.microsoft.com/en-us/samples/microsoft/directx-graphics-samples/d3d12-hello-world-samples-win32/

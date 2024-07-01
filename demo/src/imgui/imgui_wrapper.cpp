// Project includes
#include "graphics/dx12_containers.h"
#include "graphics/dx12_backend.h"
#include "imgui/imgui_wrapper.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_dx12.h"
#include "imgui/imgui_impl_win32.h"

// Static descriptor heap
static ID3D12DescriptorHeap* imguiDescHeap = nullptr;

// Forward declaration
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace imgui_d3d12
{
    bool initialize_imgui(GraphicsDevice device, RenderWindow window)
    {
        // Convert the opaque types
        d3d12::DX12Window* dx12_window = (d3d12::DX12Window*)window;
        d3d12::DX12GraphicsDevice* dx12_device = (d3d12::DX12GraphicsDevice*)device;

        // Create the context
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

        // Create the descriptor heap
        D3D12_DESCRIPTOR_HEAP_DESC desc = {};
        desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        desc.NumDescriptors = 1;
        desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        if (dx12_device->device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&imguiDescHeap)) != S_OK)
            return false;

        ImGui_ImplWin32_Init(dx12_window->window);
        ImGui_ImplDX12_Init(dx12_device->device, 2, DXGI_FORMAT_R16G16B16A16_FLOAT, imguiDescHeap, imguiDescHeap->GetCPUDescriptorHandleForHeapStart(), imguiDescHeap->GetGPUDescriptorHandleForHeapStart());
        
        // Set the style
        ImGui::StyleColorsClassic();
        return true;
    }

    void release_imgui()
    {
        ImGui_ImplDX12_Shutdown();
        ImGui_ImplWin32_Shutdown();
        imguiDescHeap->Release();
        imguiDescHeap = nullptr;
        ImGui::DestroyContext();
    }

    void start_frame()
    {
        ImGui_ImplDX12_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
    }

    void end_frame()
    {
        ImGui::Render();
    }

    void draw_frame(CommandBuffer cmd, RenderTexture renderTexture)
    {
        d3d12::DX12CommandBuffer* dx12_cmd = (d3d12::DX12CommandBuffer*)cmd;
        d3d12::DX12RenderTexture* dx12_rt = (d3d12::DX12RenderTexture*)renderTexture;

        // Create the handle
        D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle(dx12_rt->descriptorHeap->GetCPUDescriptorHandleForHeapStart());
        rtvHandle.ptr += dx12_rt->heapOffset;

        // Set the render target and enqueue into the command list
        dx12_cmd->cmdList()->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);
        dx12_cmd->cmdList()->SetDescriptorHeaps(1, &imguiDescHeap);
        ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), dx12_cmd->cmdList());
    }

    void handle_input(RenderWindow window, const EventData& data)
    {
        d3d12::DX12Window* dx12_window = (d3d12::DX12Window*)window;
        ImGui_ImplWin32_WndProcHandler(dx12_window->window, data.data0, data.data1, data.data2);
    }
}

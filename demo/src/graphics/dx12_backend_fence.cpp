// Internal includes
#include "graphics/dx12_backend.h"
#include "graphics/dx12_containers.h"
#include "graphics/dx12_helpers.h"
#include "tools/string_utilities.h"
#include "tools/security.h"

namespace d3d12
{
    namespace fence
    {
        Fence create_fence(GraphicsDevice graphicsDevice, uint64_t initialValue)
        {
            // Cast the types
            DX12GraphicsDevice* dx12_device = (DX12GraphicsDevice*)graphicsDevice;

            // Create the host object
            ID3D12Fence* fence = nullptr;
            assert_msg(dx12_device->device->CreateFence(initialValue, D3D12_FENCE_FLAG_SHARED, IID_PPV_ARGS(&fence)) == S_OK, "Failed to create Fence");

            // Opaque cast and return
            return (Fence)fence;
        }

        void destroy_fence(Fence fence)
        {
            ID3D12Fence* dx12_fence = (ID3D12Fence*)fence;
            dx12_fence->Release();
        }

        void set_value(Fence fence, uint64_t value)
        {
            ID3D12Fence* dx12_fence = (ID3D12Fence*)fence;
            dx12_fence->Signal(value);
        }

        uint64_t get_value(Fence fence)
        {
            ID3D12Fence* dx12_fence = (ID3D12Fence*)fence;
            return dx12_fence->GetCompletedValue();
        }
    }
}

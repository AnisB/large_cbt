// Internal includes
#include "graphics/dx12_backend.h"
#include "graphics/dx12_containers.h"
#include "graphics/dx12_helpers.h"
#include "tools/string_utilities.h"
#include "tools/security.h"

namespace d3d12
{
    namespace profiling_scope
    {
        ProfilingScope create_profiling_scope(GraphicsDevice graphicsDevice)
        {
            // Grab the device
            DX12GraphicsDevice* deviceI = (DX12GraphicsDevice*)graphicsDevice;

            // Create the query heap
            D3D12_QUERY_HEAP_DESC queryHeapDesc = {};
            queryHeapDesc.Count = 2;
            queryHeapDesc.Type = D3D12_QUERY_HEAP_TYPE_TIMESTAMP;
            ID3D12QueryHeap* queryHeap;
            assert_msg(deviceI->device->CreateQueryHeap(&queryHeapDesc, IID_PPV_ARGS(&queryHeap)) == S_OK, "Failed to create query.");

            // Define the resource descriptor
            D3D12_RESOURCE_DESC resourceDescriptor = { D3D12_RESOURCE_DIMENSION_BUFFER, 0, sizeof(uint64_t) * 2, 1, 1, 1, DXGI_FORMAT_UNKNOWN, 1, 0, D3D12_TEXTURE_LAYOUT_ROW_MAJOR, D3D12_RESOURCE_FLAG_NONE };

            D3D12_HEAP_PROPERTIES heap;
            memset(&heap, 0, sizeof(heap));
            heap.Type = D3D12_HEAP_TYPE_READBACK;

            // Create the resource
            ID3D12Resource* buffer;
            assert_msg(deviceI->device->CreateCommittedResource(&heap, D3D12_HEAP_FLAG_NONE, &resourceDescriptor, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&buffer)) == S_OK, "Failed to create the graphics buffer.");
                
            // Create and fill the internal structure
            DX12Query* queryI = new DX12Query();
            queryI->heap = queryHeap;
            queryI->state = D3D12_RESOURCE_STATE_PREDICATION;
            queryI->result = buffer;

            // Convert to the opaque structure
            return (ProfilingScope)queryI;
        }

        void destroy_profiling_scope(ProfilingScope profilingScope)
        {
            DX12Query* query = (DX12Query*)profilingScope;
            query->heap->Release();
            query->result->Release();
            delete query;
        }

        uint64_t get_duration_us(ProfilingScope profilingScope, CommandQueue cmdQ, CommandBufferType type)
        {
            DX12Query* query = (DX12Query*)profilingScope;
            DX12CommandQueue* dx12_cmdQ = (DX12CommandQueue*)cmdQ;

            char* data = nullptr;
            D3D12_RANGE range = { 0, sizeof(uint64_t) * 2 };
            query->result->Map(0, &range, (void**)&data);
            uint64_t profileDuration = ((uint64_t*)data)[1] - ((uint64_t*)data)[0];
            query->result->Unmap(0, nullptr);
            return (uint64_t)(profileDuration / (double)dx12_cmdQ->directSubQueue.frequency * 1e6);
        }
    }
}

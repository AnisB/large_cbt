// Internal includes
#include "graphics/dx12_backend.h"
#include "graphics/dx12_containers.h"
#include "graphics/dx12_helpers.h"
#include "tools/string_utilities.h"
#include "tools/security.h"

namespace d3d12
{
    // Function to create the command queue
    ID3D12CommandQueue* CreateCommandQueue(ID3D12Device2* device, D3D12_COMMAND_LIST_TYPE type, D3D12_COMMAND_QUEUE_PRIORITY priority)
    {
        D3D12_COMMAND_QUEUE_DESC desc = {};
        desc.Type = type;
        desc.Priority = priority;
        desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        desc.NodeMask = 0;

        // Create the command queue and ensure it's been succesfully created
        ID3D12CommandQueue* d3d12CommandQueue;
        assert_msg(device->CreateCommandQueue(&desc, IID_PPV_ARGS(&d3d12CommandQueue)) == S_OK, "Command queue creation failed.");

        // Return the command queue
        return d3d12CommandQueue;
    }

    void create_sub_command_queue(DX12GraphicsDevice* dx12_device, DX12CommandSubQueue& subQueue, D3D12_COMMAND_LIST_TYPE type, CommandQueuePriority priority)
    {
        subQueue.priority = convert_command_queue_priority(priority);
        subQueue.queue = CreateCommandQueue(dx12_device->device, type, subQueue.priority);
        assert_msg(subQueue.queue != nullptr, "Failed to create the direct command queue.");
        assert_msg(subQueue.queue->GetTimestampFrequency(&subQueue.frequency) == S_OK, "Failed to get the GPU frequency.");
        assert_msg(dx12_device->device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&subQueue.fence)) == S_OK, "Failed to create Fence");
        subQueue.fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        subQueue.fenceValue = 0;
    }

    void destroy_sub_command_queue(DX12CommandSubQueue& subQueue)
    {
        subQueue.queue->Release();
        subQueue.fence->Release();
        CloseHandle(subQueue.fenceEvent);
    }

    namespace command_queue
    {
        CommandQueue create_command_queue(GraphicsDevice graphicsDevice, CommandQueuePriority directPriority, CommandQueuePriority computePriority, CommandQueuePriority copyPriority)
        {
            // Cast the types
            DX12GraphicsDevice* dx12_device = (DX12GraphicsDevice*)graphicsDevice;

            // Create the host object
            DX12CommandQueue* dx12_commandQueue = new DX12CommandQueue();
            dx12_commandQueue->deviceI = dx12_device;

            // Create the direct queue
            create_sub_command_queue(dx12_device, dx12_commandQueue->directSubQueue, D3D12_COMMAND_LIST_TYPE_DIRECT, directPriority);

            // Create the compute queue
            create_sub_command_queue(dx12_device, dx12_commandQueue->computeSubQueue, D3D12_COMMAND_LIST_TYPE_COMPUTE, computePriority);

            // Create the copy queue
            create_sub_command_queue(dx12_device, dx12_commandQueue->copySubQueue, D3D12_COMMAND_LIST_TYPE_COPY, copyPriority);

            // Opaque cast and return
            return (CommandQueue)dx12_commandQueue;
        }

        void destroy_command_queue(CommandQueue commandQueue)
        {
            DX12CommandQueue* dx12_commandQueue = (DX12CommandQueue*)commandQueue;
            destroy_sub_command_queue(dx12_commandQueue->directSubQueue);
            destroy_sub_command_queue(dx12_commandQueue->computeSubQueue);
            destroy_sub_command_queue(dx12_commandQueue->copySubQueue);
            delete dx12_commandQueue;
        }

        void execute_command_buffer(CommandQueue commandQueue, CommandBuffer commandBuffer)
        {
            // Grab the internal structures
            DX12CommandBuffer* dx12_commandBuffer = (DX12CommandBuffer*)commandBuffer;
            DX12CommandQueue* dx12_commandQueue = (DX12CommandQueue*)commandQueue;

            ID3D12CommandList* const commandLists[] = { dx12_commandBuffer->cmdList()};
            switch (dx12_commandBuffer->type)
            {
                case D3D12_COMMAND_LIST_TYPE_DIRECT:
                    dx12_commandQueue->directSubQueue.queue->ExecuteCommandLists(1, commandLists);
                break;
                case D3D12_COMMAND_LIST_TYPE_COMPUTE:
                    dx12_commandQueue->computeSubQueue.queue->ExecuteCommandLists(1, commandLists);
                break;
                case D3D12_COMMAND_LIST_TYPE_COPY:
                    dx12_commandQueue->copySubQueue.queue->ExecuteCommandLists(1, commandLists);
                break;
            }
        }

        void signal_event_wait(DX12CommandSubQueue& subQueue)
        {
            subQueue.fenceValue++;
            subQueue.queue->Signal(subQueue.fence, subQueue.fenceValue);
            subQueue.fence->SetEventOnCompletion(subQueue.fenceValue, subQueue.fenceEvent);
            WaitForSingleObject(subQueue.fenceEvent, INFINITE);
        }
            
        void flush(CommandQueue commandQueue, CommandBufferType type)
        {
            DX12CommandQueue* dx12_commandQueue = (DX12CommandQueue*)commandQueue;

            switch (type)
            {
                case CommandBufferType::Default:
                    signal_event_wait(dx12_commandQueue->directSubQueue);
                    break;
                case CommandBufferType::Compute:
                    signal_event_wait(dx12_commandQueue->computeSubQueue);
                    break;
                case CommandBufferType::Copy:
                    signal_event_wait(dx12_commandQueue->copySubQueue);
                    break;
            }
        }

        void signal(CommandQueue commandQueue, Fence fence, uint64_t value, CommandBufferType type)
        {
            DX12CommandQueue* dx12_commandQueue = (DX12CommandQueue*)commandQueue;
            ID3D12Fence* dx12_fence = (ID3D12Fence*)fence;
            switch (type)
            {
                case CommandBufferType::Default:
                    dx12_commandQueue->directSubQueue.queue->Signal(dx12_fence, value);
                    break;
                case CommandBufferType::Compute:
                    dx12_commandQueue->computeSubQueue.queue->Signal(dx12_fence, value);
                    break;
                case CommandBufferType::Copy:
                    dx12_commandQueue->copySubQueue.queue->Signal(dx12_fence, value);
                    break;
            }
        }

        void wait(CommandQueue commandQueue, Fence fence, uint64_t value, CommandBufferType type)
        {
            DX12CommandQueue* dx12_commandQueue = (DX12CommandQueue*)commandQueue;
            ID3D12Fence* dx12_fence = (ID3D12Fence*)fence;
            switch (type)
            {
                case CommandBufferType::Default:
                    dx12_commandQueue->directSubQueue.queue->Wait(dx12_fence, value);
                    break;
                case CommandBufferType::Compute:
                    dx12_commandQueue->computeSubQueue.queue->Wait(dx12_fence, value);
                    break;
                case CommandBufferType::Copy:
                    dx12_commandQueue->copySubQueue.queue->Wait(dx12_fence, value);
                    break;
            }
        }
    }
}

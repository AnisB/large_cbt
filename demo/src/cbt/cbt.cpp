// Project includes
#include "cbt/cbt.h"
#include "graphics/dx12_backend.h"

void initialize_gpu_cbt(const CBT& cbt, GraphicsDevice device, CommandQueue queue, CommandBuffer buffer, GPU_CBT& gpuCBT)
{
    // CBT upload buffers
    GraphicsBuffer cbtUploadBuffer[2] = { 0, 0 };

    // We reset the command buffer
    d3d12::command_buffer::reset(buffer);

    // Create the graphics buffer to upload, process and readback the bitfield buffer
    gpuCBT.bufferCount = cbt.num_internal_buffers();
    gpuCBT.lastLevelSize = cbt.last_level_size();
    for (uint32_t bufferIdx = 0; bufferIdx < gpuCBT.bufferCount; ++bufferIdx)
    {
        uint32_t bufferSize = cbt.buffer_size(bufferIdx);
        uint32_t elementSize = cbt.element_size(bufferIdx);
        gpuCBT.bufferArray[bufferIdx] = d3d12::graphics_resources::create_graphics_buffer(device, bufferSize, elementSize, GraphicsBufferType::Default);
        {
            cbtUploadBuffer[bufferIdx] = d3d12::graphics_resources::create_graphics_buffer(device, bufferSize, elementSize, GraphicsBufferType::Upload);
            d3d12::graphics_resources::set_buffer_data(cbtUploadBuffer[bufferIdx], cbt.raw_buffer(bufferIdx), bufferSize);
            d3d12::command_buffer::copy_graphics_buffer(buffer, cbtUploadBuffer[bufferIdx], gpuCBT.bufferArray[bufferIdx]);
        }
    }

    // Execute and flush the queue
    d3d12::command_buffer::close(buffer);
    d3d12::command_queue::execute_command_buffer(queue, buffer);
    d3d12::command_queue::flush(queue);

    // Make sure to free the temporary graphics buffers
    for (uint32_t bufferIdx = 0; bufferIdx < gpuCBT.bufferCount; ++bufferIdx)
        d3d12::graphics_resources::destroy_graphics_buffer(cbtUploadBuffer[bufferIdx]);
}

void release_gpu_cbt(GPU_CBT& gpuCBT)
{
    // Destroy the buffers
    for (uint32_t bufferIdx = 0; bufferIdx < gpuCBT.bufferCount; ++bufferIdx)
        d3d12::graphics_resources::destroy_graphics_buffer(gpuCBT.bufferArray[bufferIdx]);

    // Reset the values
    gpuCBT.numElements = 0;
    gpuCBT.bufferCount = 0;
    gpuCBT.lastLevelSize = 0;
    gpuCBT.bufferArray[0] = 0;
    gpuCBT.bufferArray[1] = 0;
}


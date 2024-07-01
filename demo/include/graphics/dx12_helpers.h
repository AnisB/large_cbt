#pragma once

// Project includes
#include "graphics/dx12_backend.h"
#include "graphics/dx12_containers.h"

namespace d3d12
{
    // Textures
    DXGI_FORMAT format_to_dxgi_format(TextureFormat textureFormat);
    DXGI_FORMAT sanitize_dxgi_format_clear(DXGI_FORMAT format);
    DXGI_FORMAT sanitize_dxgi_format_srv(DXGI_FORMAT format);
    D3D12_RESOURCE_DIMENSION texture_dimension_to_dx12_resource_dimension(TextureType type);
    bool is_depth_format(TextureFormat format);
    uint8_t format_alignment(TextureFormat format);
    D3D12_FILTER filter_mode_to_dxgi_filter(FilterMode mode);

    // Command
    D3D12_COMMAND_LIST_TYPE convert_command_buffer_type(CommandBufferType type);
    D3D12_COMMAND_QUEUE_PRIORITY convert_command_queue_priority(CommandQueuePriority priority);

    // Descriptor heaps
    ID3D12DescriptorHeap* create_descriptor_heap_internal(DX12GraphicsDevice* deviceI, uint32_t numDescriptors, uint32_t opaqueType);
    DX12DescriptorHeap create_descriptor_heap_suc(DX12GraphicsDevice* deviceI, uint32_t srvCount, uint32_t uavCount, uint32_t cbvCount);
    DX12DescriptorHeap create_descriptor_heap_sampler(DX12GraphicsDevice* deviceI, uint32_t samplerCount);
    void destroy_descriptor_heap(DX12DescriptorHeap& descriptorHeap);

    // Root signature
    DX12RootSignature* create_root_signature(DX12GraphicsDevice* device, uint32_t srvCount, uint32_t uavCount, uint32_t cbvCount, uint32_t samplerCount);
    void destroy_root_signature(DX12RootSignature* rootSignature);

    // Compute shaders
    void validate_compute_shader_heap(DX12ComputeShader* computeShader, uint32_t cmdBatchIndex);

    // Graphics pipeline
    void validate_graphics_pipeline_heap(DX12GraphicsPipeline* graphicsPipeline, uint32_t cmdBatchIndex);

    // Graphics device
    uint32_t vendor_to_vendor_id(GPUVendor vendor);
    GPUVendor vendor_id_to_vendor(uint32_t vendorID);
}

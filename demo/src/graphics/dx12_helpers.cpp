// Project includes
#include "graphics/dx12_helpers.h"
#include "tools/security.h"

namespace d3d12
{
    DXGI_FORMAT format_to_dxgi_format(TextureFormat format)
    {
        switch (format)
        {
            // 8 Formats
            case TextureFormat::R8_SNorm:
                return DXGI_FORMAT_R8_SNORM;
            case TextureFormat::R8_UNorm:
                return DXGI_FORMAT_R8_UNORM;
            case TextureFormat::R8_SInt:
                return DXGI_FORMAT_R8_SINT;
            case TextureFormat::R8_UInt:
                return DXGI_FORMAT_R8_UINT;
            case TextureFormat::R8G8_SNorm:
                return DXGI_FORMAT_R8G8_SNORM;
            case TextureFormat::R8G8_UNorm:
                return DXGI_FORMAT_R8G8_UNORM;
            case TextureFormat::R8G8_SInt:
                return DXGI_FORMAT_R8G8_SINT;
            case TextureFormat::R8G8_UInt:
                return DXGI_FORMAT_R8G8_UINT;
            case TextureFormat::R8G8B8A8_SNorm:
                return DXGI_FORMAT_R8G8B8A8_SNORM;
            case TextureFormat::R8G8B8A8_UNorm:
                return DXGI_FORMAT_R8G8B8A8_UNORM;
            case TextureFormat::R8G8B8A8_UNorm_SRGB:
                return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
            case TextureFormat::R8G8B8A8_UInt:
                return DXGI_FORMAT_R8G8B8A8_UINT;
            case TextureFormat::R8G8B8A8_SInt:
                return DXGI_FORMAT_R8G8B8A8_SINT;

            // 16 Formats
            case TextureFormat::R16_Float:
                return DXGI_FORMAT_R16_FLOAT;
            case TextureFormat::R16_SInt:
                return DXGI_FORMAT_R16_SINT;
            case TextureFormat::R16_UInt:
                return DXGI_FORMAT_R16_UINT;
            case TextureFormat::R16G16_Float:
                return DXGI_FORMAT_R16G16_FLOAT;
            case TextureFormat::R16G16_SInt:
                return DXGI_FORMAT_R16G16_SINT;
            case TextureFormat::R16G16_UInt:
                return DXGI_FORMAT_R16G16_UINT;
            case TextureFormat::R16G16B16A16_Float:
                return DXGI_FORMAT_R16G16B16A16_FLOAT;
            case TextureFormat::R16G16B16A16_UInt:
                return DXGI_FORMAT_R16G16B16A16_UINT;
            case TextureFormat::R16G16B16A16_SInt:
                return DXGI_FORMAT_R16G16B16A16_SINT;

            // 32 FORMATS
            case TextureFormat::R32_Float:
                return DXGI_FORMAT_R32_FLOAT;
            case TextureFormat::R32_SInt:
                return DXGI_FORMAT_R32_SINT;
            case TextureFormat::R32_UInt:
                return DXGI_FORMAT_R32_UINT;
            case TextureFormat::R32G32_Float:
                return DXGI_FORMAT_R32G32_FLOAT;
            case TextureFormat::R32G32_SInt:
                return DXGI_FORMAT_R32G32_SINT;
            case TextureFormat::R32G32_UInt:
                return DXGI_FORMAT_R32G32_UINT;
            case TextureFormat::R32G32B32A32_Float:
                return DXGI_FORMAT_R32G32B32A32_FLOAT;
            case TextureFormat::R32G32B32A32_UInt:
                return DXGI_FORMAT_R32G32B32A32_UINT;
            case TextureFormat::R32G32B32A32_SInt:
                return DXGI_FORMAT_R32G32B32A32_SINT;

            // Depth/Stencil formats
            case TextureFormat::Depth32:
                return DXGI_FORMAT_R32_TYPELESS;
            case TextureFormat::Depth24Stencil8:
                return DXGI_FORMAT_R24G8_TYPELESS;
            case TextureFormat::Depth32Stencil8:
                return DXGI_FORMAT_R32G8X24_TYPELESS;

            // Other formats
            case TextureFormat::R10G10B10A2_UNorm:
                return DXGI_FORMAT_R10G10B10A2_UNORM;
            case TextureFormat::R10G10B10A2_UInt:
                return DXGI_FORMAT_R10G10B10A2_UINT;
            case TextureFormat::R11G11B10_Float:
                return DXGI_FORMAT_R11G11B10_FLOAT;
        }

        // Should never be here
        assert_fail_msg("Unknown DX12 Format");
        return DXGI_FORMAT_R8G8B8A8_SNORM;
    }

    DXGI_FORMAT sanitize_dxgi_format_clear(DXGI_FORMAT format)
    {
        switch (format)
        {
            case DXGI_FORMAT_R32_TYPELESS:
                return DXGI_FORMAT_D32_FLOAT;
            case DXGI_FORMAT_R24G8_TYPELESS:
                return DXGI_FORMAT_D24_UNORM_S8_UINT;
            case DXGI_FORMAT_R32G8X24_TYPELESS:
                return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
            default:
                return format;
        }
    }

    DXGI_FORMAT sanitize_dxgi_format_srv(DXGI_FORMAT format)
    {
        switch (format)
        {
            case DXGI_FORMAT_R32_TYPELESS:
                return DXGI_FORMAT_R32_FLOAT;
            case DXGI_FORMAT_R24G8_TYPELESS:
                return DXGI_FORMAT_R32_UINT;
            case DXGI_FORMAT_R32G8X24_TYPELESS:
                return DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
            default:
                return format;
        }
    }

    D3D12_RESOURCE_DIMENSION texture_dimension_to_dx12_resource_dimension(TextureType type)
    {
        switch (type)
        {
        case TextureType::Tex1D:
        case TextureType::Tex1DArray:
            return D3D12_RESOURCE_DIMENSION_TEXTURE1D;
        case TextureType::Tex2D:
        case TextureType::Tex2DArray:
        case TextureType::TexCube:
        case TextureType::TexCubeArray:
            return D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        case TextureType::Tex3D:
            return D3D12_RESOURCE_DIMENSION_TEXTURE3D;
        default:
            return D3D12_RESOURCE_DIMENSION_UNKNOWN;
        }
    }

    bool is_depth_format(TextureFormat format)
    {
        return format == TextureFormat::Depth32 || format == TextureFormat::Depth32Stencil8;
    }

    uint8_t format_alignment(TextureFormat format)
    {
        switch (format)
        {
            case TextureFormat::R8_UNorm:
            case TextureFormat::R8_SNorm:
            case TextureFormat::R8_UInt:
            case TextureFormat::R8_SInt:
                return 1;
            case TextureFormat::R8G8B8A8_SNorm:
            case TextureFormat::R8G8B8A8_UNorm:
            case TextureFormat::R8G8B8A8_UNorm_SRGB:
            case TextureFormat::R8G8B8A8_UInt:
            case TextureFormat::R8G8B8A8_SInt:
            case TextureFormat::R16G16_Float:
            case TextureFormat::R32_Float:
                return 4;

            case TextureFormat::R16G16B16A16_Float:
            case TextureFormat::R16G16B16A16_UInt:
            case TextureFormat::R16G16B16A16_SInt:
            case TextureFormat::R32G32_Float:
            case TextureFormat::Depth32Stencil8:
                return 8;

            case TextureFormat::Depth24Stencil8:
            case TextureFormat::Depth32:
                return 4;

            case TextureFormat::R32G32B32A32_Float:
            case TextureFormat::R32G32B32A32_UInt:
            case TextureFormat::R32G32B32A32_SInt:
                return 16;
        };

        assert_fail_msg("Unknown format.");
        return 0;
    }

    D3D12_FILTER filter_mode_to_dxgi_filter(FilterMode mode)
    {
        switch (mode)
        {
        case FilterMode::Point:
            return D3D12_FILTER_MIN_MAG_MIP_POINT;
        case FilterMode::Linear:
            return D3D12_FILTER_MIN_MAG_MIP_LINEAR;
        case FilterMode::Anisotropic:
            return D3D12_FILTER_ANISOTROPIC;
        }
        return D3D12_FILTER_MIN_MAG_MIP_POINT;
    }

    D3D12_COMMAND_LIST_TYPE convert_command_buffer_type(CommandBufferType type)
    {
        switch (type)
        {
        case CommandBufferType::Default:
            return D3D12_COMMAND_LIST_TYPE_DIRECT;
        case CommandBufferType::Compute:
            return D3D12_COMMAND_LIST_TYPE_COMPUTE;
        case CommandBufferType::Copy:
            return D3D12_COMMAND_LIST_TYPE_COPY;
        }
        return D3D12_COMMAND_LIST_TYPE_DIRECT;
    }

    D3D12_COMMAND_QUEUE_PRIORITY convert_command_queue_priority(CommandQueuePriority priority)
    {
        switch (priority)
        {
        case CommandQueuePriority::Normal:
            return D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
        case CommandQueuePriority::High:
            return D3D12_COMMAND_QUEUE_PRIORITY_HIGH;
        case CommandQueuePriority::Realtime:
            return D3D12_COMMAND_QUEUE_PRIORITY_GLOBAL_REALTIME;
        }
        return D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
    }

    ID3D12DescriptorHeap* create_descriptor_heap_internal(DX12GraphicsDevice* deviceI, uint32_t numDescriptors, uint32_t opaqueType)
    {
        // Get the actual type
        D3D12_DESCRIPTOR_HEAP_TYPE type = (D3D12_DESCRIPTOR_HEAP_TYPE)opaqueType;
        ID3D12Device1* device = deviceI->device;

        D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
        rtvHeapDesc.NumDescriptors = numDescriptors;
        rtvHeapDesc.Type = type;
        if (type == D3D12_DESCRIPTOR_HEAP_TYPE_RTV || type == D3D12_DESCRIPTOR_HEAP_TYPE_DSV)
            rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        else
            rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

        ID3D12DescriptorHeap* descriptorHeap;
        assert_msg(device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&descriptorHeap)) == S_OK, "Failed to create descriptor heap.");
        return descriptorHeap;
    }

    void destroy_descriptor_heap_internal(ID3D12DescriptorHeap* descriptorHeap)
    {
        descriptorHeap->Release();
    }

    DX12DescriptorHeap create_descriptor_heap_suc(DX12GraphicsDevice* deviceI, uint32_t srvCount, uint32_t uavCount, uint32_t cbvCount)
    {
        // Create the descriptor heap for this compute shader
        DX12DescriptorHeap descriptorHeap;
        ID3D12DescriptorHeap* descHeap = create_descriptor_heap_internal(deviceI, srvCount + uavCount + cbvCount, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        descriptorHeap.descriptorHeap = descHeap;
        descriptorHeap.type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

        // Pre-evaluate the CPU Heap handles
        uint32_t descSize = deviceI->descriptorSize[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV];
        descriptorHeap.srvCPU = descHeap->GetCPUDescriptorHandleForHeapStart();
        descriptorHeap.uavCPU = descriptorHeap.srvCPU;
        descriptorHeap.uavCPU.ptr += (uint64_t)srvCount * descSize;
        descriptorHeap.cbvCPU = descriptorHeap.uavCPU;
        descriptorHeap.cbvCPU.ptr += (uint64_t)uavCount * descSize;

        // Pre-evaluate the GPU Heap handles
        descriptorHeap.srvGPU = descHeap->GetGPUDescriptorHandleForHeapStart();
        descriptorHeap.uavGPU = descriptorHeap.srvGPU;
        descriptorHeap.uavGPU.ptr += (uint64_t)srvCount * descSize;
        descriptorHeap.cbvGPU = descriptorHeap.uavGPU;
        descriptorHeap.cbvGPU.ptr += (uint64_t)uavCount * descSize;

        return descriptorHeap;
    }

    DX12DescriptorHeap create_descriptor_heap_sampler(DX12GraphicsDevice* deviceI, uint32_t samplerCount)
    {
        // Create the descriptor heap for this compute shader
        DX12DescriptorHeap descriptorHeap;
        ID3D12DescriptorHeap* descHeap = create_descriptor_heap_internal(deviceI, samplerCount, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
        descriptorHeap.descriptorHeap = descHeap;
        descriptorHeap.type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;

        // Pre-evaluate the CPU Heap handles
        descriptorHeap.samplerCPU = descHeap->GetCPUDescriptorHandleForHeapStart();

        // Pre-evaluate the GPU Heap handles
        descriptorHeap.samplerGPU = descHeap->GetGPUDescriptorHandleForHeapStart();

        return descriptorHeap;
    }

    void destroy_descriptor_heap(DX12DescriptorHeap& descriptorHeap)
    {
        descriptorHeap.descriptorHeap->Release();
    }

    DX12RootSignature* create_root_signature(DX12GraphicsDevice* deviceI, uint32_t srvCount, uint32_t uavCount, uint32_t cbvCount, uint32_t samplerCount)
    {
        // Grab the graphics device
        ID3D12Device1* device = deviceI->device;

        // Create the root signature for the shader
        D3D12_ROOT_PARAMETER rootParameters[4];
        D3D12_DESCRIPTOR_RANGE descRange[4];

        // Create our internal structure
        DX12RootSignature* dx12_rootSignature = new DX12RootSignature();
        dx12_rootSignature->srvIndex = UINT32_MAX;
        dx12_rootSignature->uavIndex = UINT32_MAX;
        dx12_rootSignature->cbvIndex = UINT32_MAX;
        dx12_rootSignature->samplerIndex = UINT32_MAX;

        // Tracking the current count/index
        uint8_t cdIndex = 0;

        // Process the SRVs
        if (srvCount > 0)
        {
            dx12_rootSignature->srvIndex = cdIndex;
            descRange[cdIndex].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
            descRange[cdIndex].NumDescriptors = srvCount;
            descRange[cdIndex].BaseShaderRegister = 0; // t0..tN
            descRange[cdIndex].RegisterSpace = 0;
            descRange[cdIndex].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
            rootParameters[cdIndex].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
            rootParameters[cdIndex].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
            rootParameters[cdIndex].DescriptorTable.NumDescriptorRanges = 1;
            rootParameters[cdIndex].DescriptorTable.pDescriptorRanges = &descRange[cdIndex];
            cdIndex++;
        }

        // Process the UAVs
        if (uavCount > 0)
        {
            dx12_rootSignature->uavIndex = cdIndex;
            descRange[cdIndex].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
            descRange[cdIndex].NumDescriptors = uavCount;
            descRange[cdIndex].BaseShaderRegister = 0; // u0..uN
            descRange[cdIndex].RegisterSpace = 0;
            descRange[cdIndex].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
            rootParameters[cdIndex].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
            rootParameters[cdIndex].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
            rootParameters[cdIndex].DescriptorTable.NumDescriptorRanges = 1;
            rootParameters[cdIndex].DescriptorTable.pDescriptorRanges = &descRange[cdIndex];
            cdIndex++;
        }

        // Process the CBVs
        if (cbvCount > 0)
        {
            dx12_rootSignature->cbvIndex = cdIndex;
            descRange[cdIndex].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
            descRange[cdIndex].NumDescriptors = cbvCount;
            descRange[cdIndex].BaseShaderRegister = 0; // b0..bN
            descRange[cdIndex].RegisterSpace = 0;
            descRange[cdIndex].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
            rootParameters[cdIndex].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
            rootParameters[cdIndex].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
            rootParameters[cdIndex].DescriptorTable.NumDescriptorRanges = 1;
            rootParameters[cdIndex].DescriptorTable.pDescriptorRanges = &descRange[cdIndex];
            cdIndex++;
        }

        // Process the Samplers
        if (samplerCount > 0)
        {
            dx12_rootSignature->samplerIndex = cdIndex;
            descRange[cdIndex].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
            descRange[cdIndex].NumDescriptors = samplerCount;
            descRange[cdIndex].BaseShaderRegister = 0; // b0..bN
            descRange[cdIndex].RegisterSpace = 0;
            descRange[cdIndex].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
            rootParameters[cdIndex].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
            rootParameters[cdIndex].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
            rootParameters[cdIndex].DescriptorTable.NumDescriptorRanges = 1;
            rootParameters[cdIndex].DescriptorTable.pDescriptorRanges = &descRange[cdIndex];
            cdIndex++;
        }

        D3D12_ROOT_SIGNATURE_DESC desc = {};
        desc.NumParameters = cdIndex;
        desc.pParameters = rootParameters;
        desc.NumStaticSamplers = 0;
        desc.pStaticSamplers = nullptr;
        desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

        // Create the signature blob
        ID3DBlob* signatureBlob;
        assert_msg(D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, nullptr) == S_OK, "Failed to create root singnature blob.");

        // Create the root signature
        assert_msg(device->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&dx12_rootSignature->rootSignature)) == S_OK, "Failed to create root signature.");

        // Release the resources
        signatureBlob->Release();

        // Case to the opaque type
        return dx12_rootSignature;
    }

    void destroy_root_signature(DX12RootSignature* rootSignature)
    {
        // Grab the internal structure
        rootSignature->rootSignature->Release();
        delete rootSignature;
    }

    void validate_compute_shader_heap(DX12ComputeShader* computeShader, uint32_t cmdBatchIndex)
    {
        // We need to check if we've entered a new frame. If it is the case we just need to:
        //  - Update the next usable heap to the first one and we're done
        //  - Update the command buffer batch index accordinly
        // This is true because we always have at least one allocated heap.
        if (computeShader->cmdBatchIndex != cmdBatchIndex)
        {
            computeShader->nextUsableHeap = 0;
            computeShader->cmdBatchIndex = cmdBatchIndex;
            return;
        }

        // If we're tracking properly the current batch but a new heap is required, we need to allocate it.
        if (computeShader->CSUHeaps.size() == computeShader->nextUsableHeap)
        {
            // Make sure there are enough heaps
            computeShader->CSUHeaps.push_back(create_descriptor_heap_suc(computeShader->device, computeShader->srvCount, computeShader->uavCount, computeShader->cbvCount));
            computeShader->samplerHeaps.push_back(create_descriptor_heap_sampler(computeShader->device, max(computeShader->samplerCount, 1)));
        }
    }

    void validate_graphics_pipeline_heap(DX12GraphicsPipeline* graphicsPipeline, uint32_t cmdBatchIndex)
    {
        // We need to check if we've entered a new frame. If it is the case we just need to:
        //  - Update the next usable heap to the first one and we're done
        //  - Update the command buffer batch index accordinly
        // This is true because we always have at least one allocated heap.
        if (graphicsPipeline->cmdBatchIndex != cmdBatchIndex)
        {
            graphicsPipeline->nextUsableHeap = 0;
            graphicsPipeline->cmdBatchIndex = cmdBatchIndex;
            return;
        }

        // If we're tracking properly the current batch but a new heap is required, we need to allocate it.
        if (graphicsPipeline->CSUHeaps.size() == graphicsPipeline->nextUsableHeap)
        {
            graphicsPipeline->CSUHeaps.push_back(create_descriptor_heap_suc(graphicsPipeline->device, graphicsPipeline->srvCount, graphicsPipeline->uavCount, graphicsPipeline->cbvCount));
            graphicsPipeline->samplerHeaps.push_back(create_descriptor_heap_sampler(graphicsPipeline->device, max(graphicsPipeline->samplerCount, 1)));
        }
    }

    GPUVendor vendor_id_to_vendor(uint32_t vendorID)
    {
        switch (vendorID)
        {
        case 0x8086:
            return GPUVendor::Intel;
        case 0x1002:
            return GPUVendor::AMD;
        case 0x10de:
            return GPUVendor::Nvidia;
        default:
            return GPUVendor::Other;
        }
    }

    uint32_t vendor_to_vendor_id(GPUVendor vendor)
    {
        switch (vendor)
        {
        case GPUVendor::Intel:
            return 0x8086;
        case GPUVendor::AMD:
            return 0x1002;
        case GPUVendor::Nvidia:
            return 0x10de;
        default:
            return 0xffff;
        }
    }
}


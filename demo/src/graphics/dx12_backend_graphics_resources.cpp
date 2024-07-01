// Internal includes
#include "graphics/dx12_backend.h"
#include "graphics/dx12_containers.h"
#include "graphics/dx12_helpers.h"
#include "tools/security.h"
#include "tools/string_utilities.h"

namespace d3d12
{
	namespace graphics_resources
	{
		Texture create_texture(GraphicsDevice graphicsDevice, TextureType type, uint32_t width, uint32_t height, uint32_t depth, uint32_t mipCount, bool isUAV, TextureFormat format, float4 clearColor, const char* debugName)
		{
			TextureDescriptor texDescriptor;
			texDescriptor.type = type;
			texDescriptor.width = width;
			texDescriptor.height = height;
			texDescriptor.depth = depth;
			texDescriptor.mipCount = mipCount;
			texDescriptor.isUAV = isUAV;
			texDescriptor.format = format;
			texDescriptor.clearColor = clearColor;
			texDescriptor.debugName = debugName;
			return create_texture(graphicsDevice, texDescriptor);
		}

		Texture create_texture(GraphicsDevice graphicsDevice, const TextureDescriptor& rtDesc)
		{
			DX12GraphicsDevice* deviceI = (DX12GraphicsDevice*)graphicsDevice;
			ID3D12Device1* device = deviceI->device;
			assert(deviceI != nullptr);

			// Define the heap
			D3D12_HEAP_PROPERTIES heapProperties = {};
			heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
			heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
			heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

			// Not sure about this
			heapProperties.VisibleNodeMask = 1;

			D3D12_RESOURCE_DESC resourceDescriptor = {};
			resourceDescriptor.Dimension = texture_dimension_to_dx12_resource_dimension(rtDesc.type);
			resourceDescriptor.Width = rtDesc.width;
			resourceDescriptor.Height = rtDesc.height;
			resourceDescriptor.DepthOrArraySize = (uint16_t)rtDesc.depth;
			resourceDescriptor.MipLevels = (uint16_t)rtDesc.mipCount;
			resourceDescriptor.Format = format_to_dxgi_format(rtDesc.format);
			resourceDescriptor.SampleDesc = DXGI_SAMPLE_DESC{ 1, 0 };
			resourceDescriptor.Alignment = 65536;

			// This is a choice for now
			resourceDescriptor.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;

			// Raise all the relevant flags
			resourceDescriptor.Flags = D3D12_RESOURCE_FLAG_NONE;
			resourceDescriptor.Flags |= rtDesc.isUAV ? D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS : D3D12_RESOURCE_FLAG_NONE;

			// Resource states
			D3D12_RESOURCE_STATES state = rtDesc.isUAV ? D3D12_RESOURCE_STATE_UNORDERED_ACCESS : D3D12_RESOURCE_STATE_COMMON;

			// Create the actual texture
			ID3D12Resource* resource;
			assert_msg(device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDescriptor, state, nullptr, IID_PPV_ARGS(&resource)) == S_OK, "Failed to create render target.");

			// Create the render texture internal structure
			DX12Texture* dx12_texture = new DX12Texture();
			dx12_texture->resource = resource;
			dx12_texture->width = rtDesc.width;
			dx12_texture->height = rtDesc.height;
			dx12_texture->depth = rtDesc.depth;
			dx12_texture->mipLevels = rtDesc.mipCount;
			dx12_texture->format = resourceDescriptor.Format;
			dx12_texture->alignment = format_alignment(rtDesc.format);
			dx12_texture->state = state;
			dx12_texture->type = rtDesc.type;

			// Return the render target
			return (Texture)dx12_texture;
		}

		void destroy_texture(Texture texture)
		{
			DX12Texture* dx12_graphicsTexture = (DX12Texture*)texture;
			dx12_graphicsTexture->resource->Release();
		}

		void texture_dimensions(Texture texture, uint32_t& width, uint32_t& height, uint32_t& depth)
		{
			DX12Texture* dx12_graphicsTexture = (DX12Texture*)texture;
			width = dx12_graphicsTexture->width;
			height = dx12_graphicsTexture->height;
			depth = dx12_graphicsTexture->depth;
		}

		RenderTexture create_render_texture(GraphicsDevice graphicsDevice, TextureType type, uint32_t width, uint32_t height, uint32_t depth, uint32_t mipCount, bool isUAV, TextureFormat format, float4 clearColor, const char* debugName)
		{
			TextureDescriptor texDescriptor;
			texDescriptor.type = type;
			texDescriptor.width = width;
			texDescriptor.height = height;
			texDescriptor.depth = depth;
			texDescriptor.mipCount = mipCount;
			texDescriptor.isUAV = isUAV;
			texDescriptor.format = format;
			texDescriptor.clearColor = clearColor;
			texDescriptor.debugName = debugName;
			return create_render_texture(graphicsDevice, texDescriptor);
		}

		RenderTexture create_render_texture(GraphicsDevice graphicsDevice, const TextureDescriptor& rtDesc)
		{
			DX12GraphicsDevice* deviceI = (DX12GraphicsDevice*)graphicsDevice;
			ID3D12Device1* device = deviceI->device;
			assert(deviceI != nullptr);

			// Is this a regular render target or a depth stencil texture?
			bool isDepth = is_depth_format(rtDesc.format);

			// Define the heap
			D3D12_HEAP_PROPERTIES heapProperties = {};
			heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
			heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
			heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

			// Create the resource
			D3D12_RESOURCE_DESC resourceDescriptor = {};
			resourceDescriptor.Dimension = texture_dimension_to_dx12_resource_dimension(rtDesc.type);
			resourceDescriptor.Width = rtDesc.width;
			resourceDescriptor.Height = rtDesc.height;
			resourceDescriptor.DepthOrArraySize = (uint16_t)rtDesc.depth;
			resourceDescriptor.MipLevels = (uint16_t)rtDesc.mipCount;
			resourceDescriptor.Format = format_to_dxgi_format(rtDesc.format);
			resourceDescriptor.SampleDesc = DXGI_SAMPLE_DESC{ 1, 0 };
			resourceDescriptor.Alignment = 65536;

			// This is a choice for now
			resourceDescriptor.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;

			// Define the clear value
			D3D12_CLEAR_VALUE clearValue;
			clearValue.Format = sanitize_dxgi_format_clear(resourceDescriptor.Format);
			if (isDepth)
			{
				clearValue.DepthStencil.Depth = rtDesc.clearColor.x;
				clearValue.DepthStencil.Stencil = 0;
			}
			else
			{
				memcpy(clearValue.Color, &rtDesc.clearColor.x, 4 * sizeof(float));
			}

			// Raise all the relevant flags
			resourceDescriptor.Flags = D3D12_RESOURCE_FLAG_NONE;
			resourceDescriptor.Flags |= rtDesc.isUAV ? D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS : D3D12_RESOURCE_FLAG_NONE;
			resourceDescriptor.Flags |= isDepth ? D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL : D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
				
			// Resource states
			D3D12_RESOURCE_STATES state = D3D12_RESOURCE_STATE_COMMON;
			if (rtDesc.isUAV)
				state |= D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
			else
				state |= isDepth ? D3D12_RESOURCE_STATE_DEPTH_WRITE : D3D12_RESOURCE_STATE_RENDER_TARGET;

			// Create the actual texture
			ID3D12Resource* resource;
			assert_msg(device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_ALLOW_ALL_BUFFERS_AND_TEXTURES, &resourceDescriptor, state, &clearValue, IID_PPV_ARGS(&resource)) == S_OK, "Failed to create render target.");
			if (rtDesc.debugName != "")
				resource->SetName(convert_to_wide(rtDesc.debugName).c_str());

			// Create the descriptor heap for the view
			ID3D12DescriptorHeap* descHeap = create_descriptor_heap_internal(deviceI, 1, isDepth ? D3D12_DESCRIPTOR_HEAP_TYPE_DSV : D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

			// Create a depth stencil view description.
			D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilViewDsc = {};
			depthStencilViewDsc.Format = clearValue.Format;
			depthStencilViewDsc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
			depthStencilViewDsc.Texture2D.MipSlice = 0;

			// Create a render target view for it
			D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle(descHeap->GetCPUDescriptorHandleForHeapStart());
			if (isDepth)
				device->CreateDepthStencilView(resource, &depthStencilViewDsc, rtvHandle);
			else
				device->CreateRenderTargetView(resource, nullptr, rtvHandle);

			// Create the render texture internal structure
			DX12RenderTexture* dx12_renderTexture = new DX12RenderTexture();
			dx12_renderTexture->texture.resource = resource;
			dx12_renderTexture->texture.width = rtDesc.width;
			dx12_renderTexture->texture.height = rtDesc.height;
			dx12_renderTexture->texture.depth = rtDesc.depth;
			dx12_renderTexture->texture.mipLevels = rtDesc.mipCount;
			dx12_renderTexture->texture.format = resourceDescriptor.Format;
			dx12_renderTexture->texture.alignment = format_alignment(rtDesc.format);
			dx12_renderTexture->texture.state = state;
			dx12_renderTexture->texture.type = rtDesc.type;
			dx12_renderTexture->texture.isDepth = isDepth;
			dx12_renderTexture->descriptorHeap = descHeap;
			dx12_renderTexture->heapOffset = 0;

			// Return the render target
			return (RenderTexture)dx12_renderTexture;
		}

		void destroy_render_texture(RenderTexture renderTexture)
		{
			DX12RenderTexture* dx12_graphicsTexture = (DX12RenderTexture*)renderTexture;
			dx12_graphicsTexture->descriptorHeap->Release();
			dx12_graphicsTexture->texture.resource->Release();
		}

		void render_texture_dimensions(RenderTexture renderTexture, uint32_t& width, uint32_t& height, uint32_t& depth)
		{
			DX12RenderTexture* dx12_graphicsTexture = (DX12RenderTexture*)renderTexture;
			width = dx12_graphicsTexture->texture.width;
			height = dx12_graphicsTexture->texture.height;
			depth = dx12_graphicsTexture->texture.depth;
		}

		GraphicsBuffer create_graphics_buffer(GraphicsDevice graphicsDevice, uint64_t bufferSize, uint32_t elementSize, GraphicsBufferType bufferType)
		{
			DX12GraphicsDevice* deviceI = (DX12GraphicsDevice*)graphicsDevice;

			// Define the heap
			D3D12_HEAP_PROPERTIES heapProperties = {};
			heapProperties.Type = (bufferType == GraphicsBufferType::Default || bufferType == GraphicsBufferType::RTAS) ? D3D12_HEAP_TYPE_DEFAULT : (bufferType == GraphicsBufferType::Upload ? D3D12_HEAP_TYPE_UPLOAD : D3D12_HEAP_TYPE_READBACK);
			heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
			heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
			heapProperties.CreationNodeMask = 0;
			heapProperties.VisibleNodeMask = 0;

			// Define the resource descriptor
			D3D12_RESOURCE_DESC resourceDescriptor;
			resourceDescriptor.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
			resourceDescriptor.Alignment = 0;
			resourceDescriptor.Width = bufferSize;
			resourceDescriptor.Height = 1;
			resourceDescriptor.DepthOrArraySize = 1;
			resourceDescriptor.MipLevels = 1;
			resourceDescriptor.Format = DXGI_FORMAT_UNKNOWN;
			resourceDescriptor.SampleDesc.Count = 1;
			resourceDescriptor.SampleDesc.Quality = 0;
			resourceDescriptor.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
			resourceDescriptor.Flags = (bufferType == GraphicsBufferType::Default || bufferType == GraphicsBufferType::RTAS ) ? D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS : D3D12_RESOURCE_FLAG_NONE;
			resourceDescriptor.Flags |= (bufferType == GraphicsBufferType::RTAS ? D3D12_RESOURCE_FLAG_RAYTRACING_ACCELERATION_STRUCTURE : D3D12_RESOURCE_FLAG_NONE);

			// Resource states
			D3D12_RESOURCE_STATES state = D3D12_RESOURCE_STATE_COMMON;
			if (bufferType == GraphicsBufferType::Upload)
				state = D3D12_RESOURCE_STATE_GENERIC_READ;
			else if (bufferType == GraphicsBufferType::Readback)
				state = D3D12_RESOURCE_STATE_COPY_DEST;
			else if (bufferType == GraphicsBufferType::RTAS)
				state = D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;

			// Create the resource
			ID3D12Resource* buffer;
			assert_msg(deviceI->device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDescriptor, state, nullptr, IID_PPV_ARGS(&buffer)) == S_OK, "Failed to create the graphics buffer.");

			// Create the buffer internal structure
			DX12GraphicsBuffer* dx12_graphicsBuffer = new DX12GraphicsBuffer();
			dx12_graphicsBuffer->resource = buffer;
			dx12_graphicsBuffer->state = state;
			dx12_graphicsBuffer->heapType = bufferType;
			dx12_graphicsBuffer->bufferSize = bufferSize;
			dx12_graphicsBuffer->elementSize = (uint32_t)elementSize;
				
			// Return the opaque structure
			return (GraphicsBuffer)dx12_graphicsBuffer;
		}

		void destroy_graphics_buffer(GraphicsBuffer graphicsBuffer)
		{
			DX12GraphicsBuffer* dx12_buffer = (DX12GraphicsBuffer*)graphicsBuffer;
			dx12_buffer->resource->Release();
			delete dx12_buffer;
		}

		void set_buffer_data(GraphicsBuffer graphicsBuffer, const char* buffer, uint64_t bufferSize, uint32_t bufferOffset)
		{
			// Convert to the internal structure 
			DX12GraphicsBuffer* dx12_buffer = (DX12GraphicsBuffer*)graphicsBuffer;

			// If this is not an upload buffer, we can't do anything here
			assert(dx12_buffer->heapType == GraphicsBufferType::Upload);

			// Copy to the CPU buffer
			uint8_t* cpuBuffer = nullptr;
			D3D12_RANGE range = { 0, dx12_buffer->bufferSize };
			dx12_buffer->resource->Map(0, &range, reinterpret_cast<void **>(&cpuBuffer));
			memcpy(cpuBuffer + bufferOffset, buffer, bufferSize);
			dx12_buffer->resource->Unmap(0, nullptr);
		}

		char* allocate_cpu_buffer(GraphicsBuffer graphicsBuffer)
		{
			// Get the actual resource
			DX12GraphicsBuffer* dx12_buffer = (DX12GraphicsBuffer*)graphicsBuffer;

			// If this is not a readback or upload buffer, just stop
			if (dx12_buffer->heapType == GraphicsBufferType::Default)
				return nullptr;

			// Map it
			char* data = nullptr;
			D3D12_RANGE range = { 0, dx12_buffer->bufferSize};
			dx12_buffer->resource->Map(0, &range, (void**)&data);
			return data;
		}

		void release_cpu_buffer(GraphicsBuffer graphicsBuffer)
		{
			DX12GraphicsBuffer* dx12_buffer = (DX12GraphicsBuffer*)graphicsBuffer;
			dx12_buffer->resource->Unmap(0, nullptr);
		}

		void set_buffer_debug_name(GraphicsBuffer graphicsBuffer, const char* name)
		{
			DX12GraphicsBuffer* dx12_buffer = (DX12GraphicsBuffer*)graphicsBuffer;
			std::wstring wname = convert_to_wide(name);
			dx12_buffer->resource->SetName(wname.c_str());
		}

		ConstantBuffer create_constant_buffer(GraphicsDevice graphicsDevice, uint32_t elementSize, ConstantBufferType bufferType)
		{
			// The size needs to be aligned on 256
			uint32_t realElementSize = ((elementSize + (DX12_CB_ALIGNMENT_SIZE - 1)) / DX12_CB_ALIGNMENT_SIZE) * DX12_CB_ALIGNMENT_SIZE;

			DX12ConstantBuffer* dx12_cb = new DX12ConstantBuffer();
			dx12_cb->type = bufferType;
			dx12_cb->elementSize = realElementSize;
			dx12_cb->instanceIdx = 0;

			// Allocate the main buffer type
			dx12_cb->mainBuffer = (DX12GraphicsBuffer*)create_graphics_buffer(graphicsDevice, realElementSize, realElementSize, bufferType == ConstantBufferType::Static ? GraphicsBufferType::Upload : GraphicsBufferType::Default);

			// If the buffer is mixed, we need to allocate the intermediate buffers
			if (bufferType == ConstantBufferType::Mixed)
			{
				for (uint32_t instanceIdx = 0; instanceIdx < DX12_NUM_FRAMES; ++instanceIdx)
					dx12_cb->intermediateBuffers_internal[instanceIdx] = (DX12GraphicsBuffer*)create_graphics_buffer(graphicsDevice, realElementSize, realElementSize, GraphicsBufferType::Upload);
			}

			// return the total structure
			return (ConstantBuffer)dx12_cb;
		}

		void destroy_constant_buffer(ConstantBuffer constantBuffer)
		{
			DX12ConstantBuffer* dx12_cb = (DX12ConstantBuffer*)constantBuffer;
			if (dx12_cb->type == ConstantBufferType::Mixed)
			{
				for (uint32_t instanceIdx = 0; instanceIdx < DX12_NUM_FRAMES; ++instanceIdx)
				{
					destroy_graphics_buffer((GraphicsBuffer)dx12_cb->intermediateBuffers_internal[instanceIdx]);
				}
			}
			destroy_graphics_buffer((GraphicsBuffer)dx12_cb->mainBuffer);
			delete dx12_cb;
		}

		void set_constant_buffer(ConstantBuffer constantBuffer, const char* bufferData, uint32_t bufferSize)
		{
			// Cast to the internal type
			DX12ConstantBuffer* dx12_CB = (DX12ConstantBuffer*)constantBuffer;

			// Make sure the buffer has a static component
			assert(((uint32_t)dx12_CB->type & (uint32_t)ConstantBufferType::Static) != 0);

			// Define the sub-resource
			D3D12_RANGE readRange = {0, dx12_CB->elementSize};
			DX12GraphicsBuffer* currentGB = dx12_CB->type == ConstantBufferType::Mixed ? dx12_CB->intermediateBuffer() : dx12_CB->mainBuffer;

			// Bind it
			uint8_t* cbvDataBegin;
			currentGB->resource->Map(0, &readRange, reinterpret_cast<void**>(&cbvDataBegin));

			// Copy the memory
			memcpy(cbvDataBegin, bufferData, bufferSize);

			// Unbind it
			currentGB->resource->Unmap(0, nullptr);
		}

		BottomLevelAS create_blas(GraphicsDevice device, GraphicsBuffer vertexBuffer, uint32_t vertexCount, GraphicsBuffer indexBuffer, uint32_t numTriangles)
		{
			// Grab the graphics device
			DX12GraphicsDevice* dx12_device = (DX12GraphicsDevice*)device;
			assert_msg(dx12_device->supportRayTracing, "Ray Tracing not-supported on this device.");

			// Create the acceleration structure
			DX12BLAS* dx12_blas = new DX12BLAS();

			// Cast the resources
			dx12_blas->vertexBuffer = (DX12GraphicsBuffer*)vertexBuffer;
			dx12_blas->indexBuffer = (DX12GraphicsBuffer*)indexBuffer;

			// Describe the geometry
			dx12_blas->geometryDesc = {};
			dx12_blas->geometryDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;

			dx12_blas->geometryDesc.Triangles.IndexBuffer = dx12_blas->indexBuffer->resource->GetGPUVirtualAddress();
			dx12_blas->geometryDesc.Triangles.IndexCount = numTriangles * 3;
			dx12_blas->geometryDesc.Triangles.IndexFormat = DXGI_FORMAT_R32_UINT;
			dx12_blas->geometryDesc.Triangles.Transform3x4 = 0;

			dx12_blas->geometryDesc.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;
			dx12_blas->geometryDesc.Triangles.VertexCount = vertexCount;
			dx12_blas->geometryDesc.Triangles.VertexBuffer.StartAddress = dx12_blas->vertexBuffer->resource->GetGPUVirtualAddress();
			dx12_blas->geometryDesc.Triangles.VertexBuffer.StrideInBytes = sizeof(float3);
			dx12_blas->geometryDesc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;

			// Get required sizes for an acceleration structure.
			dx12_blas->inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
			dx12_blas->inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_NONE;
			dx12_blas->inputs.NumDescs = 1;
			dx12_blas->inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
			dx12_blas->inputs.pGeometryDescs = &dx12_blas->geometryDesc;
			dx12_device->device->GetRaytracingAccelerationStructurePrebuildInfo(&dx12_blas->inputs, &dx12_blas->preBuildInfo);
			assert(dx12_blas->preBuildInfo.ResultDataMaxSizeInBytes > 0);

			// Create the acceleration structures
			dx12_blas->data = (DX12GraphicsBuffer*)graphics_resources::create_graphics_buffer(device, dx12_blas->preBuildInfo.ResultDataMaxSizeInBytes, 4, GraphicsBufferType::RTAS);
			dx12_blas->data->resource->SetName(L"BLAS Buffer");
			dx12_blas->scratchBuffer = (DX12GraphicsBuffer*)d3d12::graphics_resources::create_graphics_buffer(device, dx12_blas->preBuildInfo.ScratchDataSizeInBytes, 4);
			dx12_blas->scratchBuffer->resource->SetName(L"TLAS Scratch Buffer");

			// return the acceleration structure
			return (BottomLevelAS)dx12_blas;
		}

		void destroy_blas(BottomLevelAS blas)
		{
			DX12BLAS* dx12_blas = (DX12BLAS*)blas;
			destroy_graphics_buffer((GraphicsBuffer)dx12_blas->scratchBuffer);
			destroy_graphics_buffer((GraphicsBuffer)dx12_blas->data);
			delete dx12_blas;
		}

		TopLevelAS create_tlas(GraphicsDevice device, uint32_t numBLAS)
		{
			// Grab the graphics device
			DX12GraphicsDevice* dx12_device = (DX12GraphicsDevice*)device;
			assert_msg(dx12_device->supportRayTracing, "Ray Tracing not-supported on this device.");

			// Create the acceleration structure
			DX12TLAS* dx12_tlas = new DX12TLAS();

			// Get required sizes for an acceleration structure.
			dx12_tlas->inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
			dx12_tlas->inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_NONE;
			dx12_tlas->inputs.NumDescs = numBLAS;
			dx12_tlas->inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
			dx12_device->device->GetRaytracingAccelerationStructurePrebuildInfo(&dx12_tlas->inputs, &dx12_tlas->preBuildInfo);
			assert(dx12_tlas->preBuildInfo.ResultDataMaxSizeInBytes > 0);
			
			// Create the main buffer
			dx12_tlas->data = (DX12GraphicsBuffer*)graphics_resources::create_graphics_buffer(device, dx12_tlas->preBuildInfo.ResultDataMaxSizeInBytes, 4, GraphicsBufferType::RTAS);
			dx12_tlas->data->resource->SetName(L"TLAS Buffer");

			// Create the scratch buffer
			dx12_tlas->scratchBuffer = (DX12GraphicsBuffer*)d3d12::graphics_resources::create_graphics_buffer(device, dx12_tlas->preBuildInfo.ScratchDataSizeInBytes, 4);
			dx12_tlas->scratchBuffer->resource->SetName(L"TLAS Scratch Buffer");
			
			// Create an instance desc for the bottom-level acceleration structure.
			dx12_tlas->instanceBufer = (DX12GraphicsBuffer*)graphics_resources::create_graphics_buffer(device, sizeof(D3D12_RAYTRACING_INSTANCE_DESC) * numBLAS, sizeof(D3D12_RAYTRACING_INSTANCE_DESC), GraphicsBufferType::Upload);
			dx12_tlas->scratchBuffer->resource->SetName(L"TLAS Instance Buffer");
			dx12_tlas->inputs.InstanceDescs = dx12_tlas->instanceBufer->resource->GetGPUVirtualAddress();
			dx12_tlas->instanceArray.resize(numBLAS);

			// return the acceleration structure
			return (TopLevelAS)dx12_tlas;
		}

		void destroy_tlas(TopLevelAS tlas)
		{
			DX12TLAS* dx12_tlas = (DX12TLAS*)tlas;
			destroy_graphics_buffer((GraphicsBuffer)dx12_tlas->data);
			destroy_graphics_buffer((GraphicsBuffer)dx12_tlas->scratchBuffer);
			destroy_graphics_buffer((GraphicsBuffer)dx12_tlas->instanceBufer);
			delete dx12_tlas;
		}

		void set_tlas_instance(TopLevelAS tlas, BottomLevelAS blas, uint32_t index)
		{
			// Cast the types
			DX12TLAS* dx12_tlas = (DX12TLAS*)tlas;
			DX12BLAS* dx12_blas = (DX12BLAS*)blas;

			// Fill the descriptor
			D3D12_RAYTRACING_INSTANCE_DESC instanceDesc = {};
			instanceDesc.Transform[0][0] = instanceDesc.Transform[1][1] = instanceDesc.Transform[2][2] = 1;
			instanceDesc.InstanceMask = 1;
			instanceDesc.AccelerationStructure = dx12_blas->data->resource->GetGPUVirtualAddress();

			// Set the descriptor
			dx12_tlas->instanceArray[index] = instanceDesc;
		}

		void upload_tlas_instance_data(TopLevelAS tlas)
		{
			DX12TLAS* dx12_tlas = (DX12TLAS*)tlas;
			set_buffer_data((GraphicsBuffer)dx12_tlas->instanceBufer, (char*)dx12_tlas->instanceArray.data(), dx12_tlas->instanceArray.size() * sizeof(D3D12_RAYTRACING_INSTANCE_DESC));
		}
	}
}

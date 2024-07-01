// Internal includes
#include "graphics/dx12_backend.h"
#include "graphics/dx12_containers.h"
#include "graphics/dx12_helpers.h"
#include "tools/security.h"

#undef max
#undef min

// System includes
#include "pix_win.h"

namespace d3d12
{
	// Command Buffer API
	namespace command_buffer
	{
		void direct_change_resource_state(DX12CommandBuffer* commandBuffer, ID3D12Resource* resource, D3D12_RESOURCE_STATES& resourceState, D3D12_RESOURCE_STATES targetState)
		{
			if (targetState != resourceState)
			{
				// Define a barrier for the resource
				D3D12_RESOURCE_BARRIER barrier = {};
				barrier.Type = (D3D12_RESOURCE_BARRIER_TYPE_TRANSITION);
				barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
				barrier.Transition.pResource = resource;
				barrier.Transition.StateBefore = resourceState;
				barrier.Transition.StateAfter = targetState;
				barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
				commandBuffer->cmdList()->ResourceBarrier(1, &barrier);
			}

			// Keep track of the new state
			resourceState = targetState;
		}

		void direct_change_resource_state(DX12CommandBuffer* commandBuffer, 
			ID3D12Resource* resourceA, D3D12_RESOURCE_STATES& resourceStateA, D3D12_RESOURCE_STATES targetStateA,
			ID3D12Resource* resourceB, D3D12_RESOURCE_STATES& resourceStateB, D3D12_RESOURCE_STATES targetStateB)
		{
			uint32_t idx = 0;
			D3D12_RESOURCE_BARRIER barriers[2];

			// Process the first barrier
			if (targetStateA != resourceStateA)
			{
				// Define a barrier for the resource
				barriers[idx].Type = (D3D12_RESOURCE_BARRIER_TYPE_TRANSITION);
				barriers[idx].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
				barriers[idx].Transition.pResource = resourceA;
				barriers[idx].Transition.StateBefore = resourceStateA;
				barriers[idx].Transition.StateAfter = targetStateA;
				barriers[idx].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
				idx++;
			}
			resourceStateA = targetStateA;

			// Process the first barrier
			if (targetStateB != resourceStateB)
			{
				// Define a barrier for the resource
				barriers[idx].Type = (D3D12_RESOURCE_BARRIER_TYPE_TRANSITION);
				barriers[idx].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
				barriers[idx].Transition.pResource = resourceB;
				barriers[idx].Transition.StateBefore = resourceStateB;
				barriers[idx].Transition.StateAfter = targetStateB;
				barriers[idx].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
				idx++;
			}
			resourceStateB = targetStateB;

			// Flush the barriers
			if (idx != 0)
				commandBuffer->cmdList()->ResourceBarrier(idx, barriers);
		}

		void direct_change_resource_state(DX12CommandBuffer* commandBuffer,
			ID3D12Resource* resourceA, D3D12_RESOURCE_STATES& resourceStateA, D3D12_RESOURCE_STATES targetStateA,
			ID3D12Resource* resourceB, D3D12_RESOURCE_STATES& resourceStateB, D3D12_RESOURCE_STATES targetStateB,
			ID3D12Resource* resourceC, D3D12_RESOURCE_STATES& resourceStateC, D3D12_RESOURCE_STATES targetStateC)
		{
			uint32_t idx = 0;
			D3D12_RESOURCE_BARRIER barriers[3];

			// Process the first barrier
			if (targetStateA != resourceStateA)
			{
				// Define a barrier for the resource
				barriers[idx].Type = (D3D12_RESOURCE_BARRIER_TYPE_TRANSITION);
				barriers[idx].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
				barriers[idx].Transition.pResource = resourceA;
				barriers[idx].Transition.StateBefore = resourceStateA;
				barriers[idx].Transition.StateAfter = targetStateA;
				barriers[idx].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
				idx++;
			}
			resourceStateA = targetStateA;

			// Process the second barrier
			if (targetStateB != resourceStateB)
			{
				// Define a barrier for the resource
				barriers[idx].Type = (D3D12_RESOURCE_BARRIER_TYPE_TRANSITION);
				barriers[idx].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
				barriers[idx].Transition.pResource = resourceB;
				barriers[idx].Transition.StateBefore = resourceStateB;
				barriers[idx].Transition.StateAfter = targetStateB;
				barriers[idx].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
				idx++;
			}
			resourceStateB = targetStateB;

			// Process the third barrier
			if (targetStateC != resourceStateC)
			{
				// Define a barrier for the resource
				barriers[idx].Type = (D3D12_RESOURCE_BARRIER_TYPE_TRANSITION);
				barriers[idx].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
				barriers[idx].Transition.pResource = resourceC;
				barriers[idx].Transition.StateBefore = resourceStateC;
				barriers[idx].Transition.StateAfter = targetStateC;
				barriers[idx].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
				idx++;
			}
			resourceStateC = targetStateC;

			// Flush the barriers
			if (idx != 0)
				commandBuffer->cmdList()->ResourceBarrier(idx, barriers);
		}

		void direct_change_resource_state(DX12CommandBuffer* commandBuffer,
			ID3D12Resource* resourceA, D3D12_RESOURCE_STATES& resourceStateA, D3D12_RESOURCE_STATES targetStateA,
			ID3D12Resource* resourceB, D3D12_RESOURCE_STATES& resourceStateB, D3D12_RESOURCE_STATES targetStateB,
			ID3D12Resource* resourceC, D3D12_RESOURCE_STATES& resourceStateC, D3D12_RESOURCE_STATES targetStateC,
			ID3D12Resource* resourceD, D3D12_RESOURCE_STATES& resourceStateD, D3D12_RESOURCE_STATES targetStateD)
		{
			uint32_t idx = 0;
			D3D12_RESOURCE_BARRIER barriers[4];

			// Process the first barrier
			if (targetStateA != resourceStateA)
			{
				// Define a barrier for the resource
				barriers[idx].Type = (D3D12_RESOURCE_BARRIER_TYPE_TRANSITION);
				barriers[idx].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
				barriers[idx].Transition.pResource = resourceA;
				barriers[idx].Transition.StateBefore = resourceStateA;
				barriers[idx].Transition.StateAfter = targetStateA;
				barriers[idx].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
				idx++;
			}
			resourceStateA = targetStateA;

			// Process the second barrier
			if (targetStateB != resourceStateB)
			{
				// Define a barrier for the resource
				barriers[idx].Type = (D3D12_RESOURCE_BARRIER_TYPE_TRANSITION);
				barriers[idx].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
				barriers[idx].Transition.pResource = resourceB;
				barriers[idx].Transition.StateBefore = resourceStateB;
				barriers[idx].Transition.StateAfter = targetStateB;
				barriers[idx].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
				idx++;
			}
			resourceStateB = targetStateB;

			// Process the third barrier
			if (targetStateC != resourceStateC)
			{
				// Define a barrier for the resource
				barriers[idx].Type = (D3D12_RESOURCE_BARRIER_TYPE_TRANSITION);
				barriers[idx].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
				barriers[idx].Transition.pResource = resourceC;
				barriers[idx].Transition.StateBefore = resourceStateC;
				barriers[idx].Transition.StateAfter = targetStateC;
				barriers[idx].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
				idx++;
			}
			resourceStateC = targetStateC;

			// Process the third barrier
			if (targetStateD != resourceStateD)
			{
				// Define a barrier for the resource
				barriers[idx].Type = (D3D12_RESOURCE_BARRIER_TYPE_TRANSITION);
				barriers[idx].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
				barriers[idx].Transition.pResource = resourceD;
				barriers[idx].Transition.StateBefore = resourceStateD;
				barriers[idx].Transition.StateAfter = targetStateD;
				barriers[idx].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
				idx++;
			}
			resourceStateD = targetStateD;

			// Flush the barriers
			if (idx != 0)
				commandBuffer->cmdList()->ResourceBarrier(idx, barriers);
		}

		void async_change_resource_state(std::vector<D3D12_RESOURCE_BARRIER>& barriers, ID3D12Resource* resource, D3D12_RESOURCE_STATES& resourceState, D3D12_RESOURCE_STATES targetState)
		{
			if (targetState != resourceState)
			{
				// Define a barrier for the resource
				D3D12_RESOURCE_BARRIER barrier = {};
				barrier.Type = (D3D12_RESOURCE_BARRIER_TYPE_TRANSITION);
				barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
				barrier.Transition.pResource = resource;
				barrier.Transition.StateBefore = resourceState;
				barrier.Transition.StateAfter = targetState;
				barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
				barriers.push_back(barrier);
			}

			// Keep track of the new state
			resourceState = targetState;
		}

		void uav_barrier_buffer(CommandBuffer commandBuffer, GraphicsBuffer targetBuffer)
		{
			// Get the internal command buffer structure
			DX12CommandBuffer* dx12_commandBuffer = (DX12CommandBuffer*)commandBuffer;

			// Prepare the input buffer if needed
			DX12GraphicsBuffer* dx12_inputBuffer = (DX12GraphicsBuffer*)targetBuffer;

			// Define a barrier for the resource
			if (dx12_inputBuffer->state == D3D12_RESOURCE_STATE_UNORDERED_ACCESS || dx12_inputBuffer->state == D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE)
			{
				D3D12_RESOURCE_BARRIER barrier = {};
				barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
				barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
				barrier.UAV.pResource = dx12_inputBuffer->resource;
				dx12_commandBuffer->cmdList()->ResourceBarrier(1, &barrier);
			}
		}

		void uav_barrier_texture(CommandBuffer commandBuffer, Texture texture)
		{
			// Get the internal command buffer structure
			DX12CommandBuffer* dx12_commandBuffer = (DX12CommandBuffer*)commandBuffer;

			// Prepare the input buffer if needed
			DX12Texture* dx12_tex = (DX12Texture*)texture;

			// Define a barrier for the resource
			if (dx12_tex->state == D3D12_RESOURCE_STATE_UNORDERED_ACCESS)
			{
				D3D12_RESOURCE_BARRIER barrier = {};
				barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
				barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
				barrier.UAV.pResource = dx12_tex->resource;
				dx12_commandBuffer->cmdList()->ResourceBarrier(1, &barrier);
			}
		}

		void uav_barrier_render_texture(CommandBuffer commandBuffer, RenderTexture renderTexture)
		{
			DX12RenderTexture* dx12_rTex = (DX12RenderTexture*)renderTexture;
			uav_barrier_texture(commandBuffer, (Texture)&(dx12_rTex->texture));
		}

		void transition_to_common(CommandBuffer commandBuffer, GraphicsBuffer targetBuffer)
		{
			// Get the internal command buffer structure
			DX12CommandBuffer* dx12_commandBuffer = (DX12CommandBuffer*)commandBuffer;

			// Prepare the input buffer if needed
			DX12GraphicsBuffer* dx12_gb = (DX12GraphicsBuffer*)targetBuffer;

			if (dx12_gb->state != D3D12_RESOURCE_STATE_COMMON)
			{
				D3D12_RESOURCE_BARRIER barrier = {};
				barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
				barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
				barrier.Transition.pResource = dx12_gb->resource;
				barrier.Transition.StateBefore = dx12_gb->state;
				barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COMMON;
				barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
				dx12_commandBuffer->cmdList()->ResourceBarrier(1, &barrier);
			}
			dx12_gb->state = D3D12_RESOURCE_STATE_COMMON;
		}

		void transition_to_copy_source(CommandBuffer commandBuffer, GraphicsBuffer targetBuffer)
		{
			// Get the internal command buffer structure
			DX12CommandBuffer* dx12_commandBuffer = (DX12CommandBuffer*)commandBuffer;

			// Prepare the input buffer if needed
			DX12GraphicsBuffer* dx12_gb = (DX12GraphicsBuffer*)targetBuffer;

			if (dx12_gb->state != D3D12_RESOURCE_STATE_COPY_SOURCE)
			{
				D3D12_RESOURCE_BARRIER barrier = {};
				barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
				barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
				barrier.Transition.pResource = dx12_gb->resource;
				barrier.Transition.StateBefore = dx12_gb->state;
				barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_SOURCE;
				barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
				dx12_commandBuffer->cmdList()->ResourceBarrier(1, &barrier);
			}
			dx12_gb->state = D3D12_RESOURCE_STATE_COPY_SOURCE;
		}

		void transition_to_present(CommandBuffer commandBuffer, RenderTexture renderTexture)
		{
			DX12CommandBuffer* dx12_commandBuffer = (DX12CommandBuffer*)commandBuffer;
			DX12RenderTexture* dx12_renderTexture = (DX12RenderTexture*)renderTexture;

			// Make sure the state is the right one
			direct_change_resource_state(dx12_commandBuffer, dx12_renderTexture->texture.resource, dx12_renderTexture->texture.state, D3D12_RESOURCE_STATE_PRESENT);
		}

		CommandBuffer create_command_buffer(GraphicsDevice graphicsDevice, CommandBufferType commandBufferType)
		{
			// Grab the graphics device
			DX12GraphicsDevice* dx12_device = (DX12GraphicsDevice*)graphicsDevice;

			// Create the command buffer
			DX12CommandBuffer* dx12_commandBuffer = new DX12CommandBuffer();
			dx12_commandBuffer->deviceI = dx12_device;
			dx12_commandBuffer->frameIdx = 0;
			dx12_commandBuffer->type = convert_command_buffer_type(commandBufferType);

			// Allocate the command list and allocators
			for (uint32_t cmdIdx = 0; cmdIdx < DX12_NUM_FRAMES; ++cmdIdx)
			{
				// Create the command allocator i
				assert_msg(dx12_device->device->CreateCommandAllocator(dx12_commandBuffer->type, IID_PPV_ARGS(&dx12_commandBuffer->commandAllocator_internal[cmdIdx])) == S_OK, "Failed to create command allocator");

				// Create the command list
				assert_msg(dx12_device->device->CreateCommandList(0, dx12_commandBuffer->type, dx12_commandBuffer->commandAllocator_internal[cmdIdx], nullptr, IID_PPV_ARGS(&dx12_commandBuffer->commandList_internal[cmdIdx])) == S_OK, "Failed to create command list.");
				assert_msg(dx12_commandBuffer->commandList_internal[cmdIdx]->Close() == S_OK, "Failed to close command list.");
			}

			// Convert to the opaque structure
			return (CommandBuffer)dx12_commandBuffer;
		}

		void destroy_command_buffer(CommandBuffer command_buffer)
		{
			// Convert to the internal structure
			DX12CommandBuffer* dx12_commandBuffer = (DX12CommandBuffer*)command_buffer;

			for (uint32_t cmdIdx = 0; cmdIdx < DX12_NUM_FRAMES; ++cmdIdx)
			{
				// Release the command list
				dx12_commandBuffer->commandList_internal[cmdIdx]->Release();

				// Release the command allocator
				dx12_commandBuffer->commandAllocator_internal[cmdIdx]->Release();
			}

			// Destroy the render environment
			delete dx12_commandBuffer;
		}

		void reset(CommandBuffer commandBuffer)
		{
			DX12CommandBuffer* dx12_commandBuffer = (DX12CommandBuffer*)commandBuffer;
			dx12_commandBuffer->frameIdx = dx12_commandBuffer->frameIdx++;
			dx12_commandBuffer->cmdAlloc()->Reset();
			dx12_commandBuffer->cmdList()->Reset(dx12_commandBuffer->cmdAlloc(), nullptr);
		}

		void close(CommandBuffer commandBuffer)
		{
			DX12CommandBuffer* dx12_commandBuffer = (DX12CommandBuffer*)commandBuffer;
			dx12_commandBuffer->cmdList()->Close();
		}

		void set_render_texture(CommandBuffer commandBuffer, RenderTexture renderTexture)
		{
			DX12CommandBuffer* dx12_commandBuffer = (DX12CommandBuffer*)commandBuffer;
			DX12RenderTexture* dx12_renderTexture = (DX12RenderTexture*)renderTexture;

			// Grab the render target view
			D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle(dx12_renderTexture->descriptorHeap->GetCPUDescriptorHandleForHeapStart());
			rtvHandle.ptr += dx12_renderTexture->heapOffset;

			direct_change_resource_state(dx12_commandBuffer, dx12_renderTexture->texture.resource, dx12_renderTexture->texture.state, D3D12_RESOURCE_STATE_RENDER_TARGET);

			// Set the render target and the current one
			dx12_commandBuffer->cmdList()->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);
		}

		void set_render_texture(CommandBuffer commandBuffer, RenderTexture renderTexture, RenderTexture depthTexture)
		{
			DX12CommandBuffer* dx12_commandBuffer = (DX12CommandBuffer*)commandBuffer;
			DX12RenderTexture* dx12_renderTexture = (DX12RenderTexture*)renderTexture;
			DX12RenderTexture* dx12_depthTexture = (DX12RenderTexture*)depthTexture;

			// Grab the render target view
			D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle(dx12_renderTexture->descriptorHeap->GetCPUDescriptorHandleForHeapStart());
			rtvHandle.ptr += dx12_renderTexture->heapOffset;
			D3D12_CPU_DESCRIPTOR_HANDLE depthvHandle(dx12_depthTexture->descriptorHeap->GetCPUDescriptorHandleForHeapStart());
			depthvHandle.ptr += dx12_depthTexture->heapOffset;

			// Flush the two barriers
			direct_change_resource_state(dx12_commandBuffer, dx12_renderTexture->texture.resource, dx12_renderTexture->texture.state, D3D12_RESOURCE_STATE_RENDER_TARGET,
				dx12_depthTexture->texture.resource, dx12_depthTexture->texture.state, D3D12_RESOURCE_STATE_DEPTH_WRITE);

			// Set the render target and the current one
			dx12_commandBuffer->cmdList()->OMSetRenderTargets(1, &rtvHandle, FALSE, &depthvHandle);
		}

		void set_render_texture(CommandBuffer commandBuffer, RenderTexture renderTexture0, RenderTexture renderTexture1, RenderTexture depthTexture)
		{
			DX12CommandBuffer* dx12_commandBuffer = (DX12CommandBuffer*)commandBuffer;
			DX12RenderTexture* dx12_renderTexture0 = (DX12RenderTexture*)renderTexture0;
			DX12RenderTexture* dx12_renderTexture1 = (DX12RenderTexture*)renderTexture1;
			DX12RenderTexture* dx12_depthTexture = (DX12RenderTexture*)depthTexture;

			// Grab the render target view
			D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle[2];
			rtvHandle[0] = D3D12_CPU_DESCRIPTOR_HANDLE(dx12_renderTexture0->descriptorHeap->GetCPUDescriptorHandleForHeapStart());
			rtvHandle[0].ptr += dx12_renderTexture0->heapOffset;
			rtvHandle[1] = D3D12_CPU_DESCRIPTOR_HANDLE(dx12_renderTexture1->descriptorHeap->GetCPUDescriptorHandleForHeapStart());
			rtvHandle[1].ptr += dx12_renderTexture1->heapOffset;
			D3D12_CPU_DESCRIPTOR_HANDLE depthvHandle(dx12_depthTexture->descriptorHeap->GetCPUDescriptorHandleForHeapStart());
			depthvHandle.ptr += dx12_depthTexture->heapOffset;

			// Flush the two barriers
			direct_change_resource_state(dx12_commandBuffer, 
				dx12_renderTexture0->texture.resource, dx12_renderTexture0->texture.state, D3D12_RESOURCE_STATE_RENDER_TARGET,
				dx12_renderTexture1->texture.resource, dx12_renderTexture1->texture.state, D3D12_RESOURCE_STATE_RENDER_TARGET,
				dx12_depthTexture->texture.resource, dx12_depthTexture->texture.state, D3D12_RESOURCE_STATE_DEPTH_WRITE);

			// Set the render target and the current one
			dx12_commandBuffer->cmdList()->OMSetRenderTargets(2, rtvHandle, FALSE, &depthvHandle);
		}

		void set_render_texture(CommandBuffer commandBuffer, RenderTexture renderTexture0, RenderTexture renderTexture1, RenderTexture renderTexture2, RenderTexture depthTexture)
		{
			DX12CommandBuffer* dx12_commandBuffer = (DX12CommandBuffer*)commandBuffer;
			DX12RenderTexture* dx12_renderTexture0 = (DX12RenderTexture*)renderTexture0;
			DX12RenderTexture* dx12_renderTexture1 = (DX12RenderTexture*)renderTexture1;
			DX12RenderTexture* dx12_renderTexture2 = (DX12RenderTexture*)renderTexture2;
			DX12RenderTexture* dx12_depthTexture = (DX12RenderTexture*)depthTexture;

			// Grab the render target view
			D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle[3];
			rtvHandle[0] = D3D12_CPU_DESCRIPTOR_HANDLE(dx12_renderTexture0->descriptorHeap->GetCPUDescriptorHandleForHeapStart());
			rtvHandle[0].ptr += dx12_renderTexture0->heapOffset;
			rtvHandle[1] = D3D12_CPU_DESCRIPTOR_HANDLE(dx12_renderTexture1->descriptorHeap->GetCPUDescriptorHandleForHeapStart());
			rtvHandle[1].ptr += dx12_renderTexture1->heapOffset;
			rtvHandle[2] = D3D12_CPU_DESCRIPTOR_HANDLE(dx12_renderTexture2->descriptorHeap->GetCPUDescriptorHandleForHeapStart());
			rtvHandle[2].ptr += dx12_renderTexture2->heapOffset;
			D3D12_CPU_DESCRIPTOR_HANDLE depthvHandle(dx12_depthTexture->descriptorHeap->GetCPUDescriptorHandleForHeapStart());
			depthvHandle.ptr += dx12_depthTexture->heapOffset;

			// Flush the two barriers
			direct_change_resource_state(dx12_commandBuffer,
				dx12_renderTexture0->texture.resource, dx12_renderTexture0->texture.state, D3D12_RESOURCE_STATE_RENDER_TARGET,
				dx12_renderTexture1->texture.resource, dx12_renderTexture1->texture.state, D3D12_RESOURCE_STATE_RENDER_TARGET,
				dx12_renderTexture2->texture.resource, dx12_renderTexture2->texture.state, D3D12_RESOURCE_STATE_RENDER_TARGET,
				dx12_depthTexture->texture.resource, dx12_depthTexture->texture.state, D3D12_RESOURCE_STATE_DEPTH_WRITE);

			// Set the render target and the current one
			dx12_commandBuffer->cmdList()->OMSetRenderTargets(3, rtvHandle, FALSE, &depthvHandle);
		}

		void clear_render_texture(CommandBuffer commandBuffer, RenderTexture renderTexture, const float4& color)
		{
			// Grab the actual structures
			DX12CommandBuffer* dx12_commandBuffer = (DX12CommandBuffer*)commandBuffer;
			DX12RenderTexture* dx12_renderTexture = (DX12RenderTexture*)renderTexture;

			// Grab the render target view
			D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle(dx12_renderTexture->descriptorHeap->GetCPUDescriptorHandleForHeapStart());
			rtvHandle.ptr += dx12_renderTexture->heapOffset;

			// Make sure the state is the right one
			direct_change_resource_state(dx12_commandBuffer, dx12_renderTexture->texture.resource, dx12_renderTexture->texture.state, D3D12_RESOURCE_STATE_RENDER_TARGET);

			// Clear with the color
			dx12_commandBuffer->cmdList()->ClearRenderTargetView(rtvHandle, &color.x, 0, nullptr);
		}

		void clear_depth_texture(CommandBuffer commandBuffer, RenderTexture depthTexture, float value)
		{
			// Grab the actual structures
			DX12CommandBuffer* dx12_commandBuffer = (DX12CommandBuffer*)commandBuffer;
			DX12RenderTexture* dx12_depthTexture = (DX12RenderTexture*)depthTexture;

			// Grab the render target view
			D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle(dx12_depthTexture->descriptorHeap->GetCPUDescriptorHandleForHeapStart());
			rtvHandle.ptr += dx12_depthTexture->heapOffset;

			// Make sure the state is the right one
			direct_change_resource_state(dx12_commandBuffer, dx12_depthTexture->texture.resource, dx12_depthTexture->texture.state, D3D12_RESOURCE_STATE_DEPTH_WRITE);

			// Clear with the color
			dx12_commandBuffer->cmdList()->ClearDepthStencilView(rtvHandle, D3D12_CLEAR_FLAGS::D3D12_CLEAR_FLAG_DEPTH, value, 0, 0, nullptr);
		}

		void clear_depth_stencil_texture(CommandBuffer commandBuffer, RenderTexture depthTexture, float depth, uint8_t stencil)
		{
			// Grab the actual structures
			DX12CommandBuffer* dx12_commandBuffer = (DX12CommandBuffer*)commandBuffer;
			DX12RenderTexture* dx12_depthTexture = (DX12RenderTexture*)depthTexture;

			// Grab the render target view
			D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle(dx12_depthTexture->descriptorHeap->GetCPUDescriptorHandleForHeapStart());
			rtvHandle.ptr += dx12_depthTexture->heapOffset;

			// Make sure the state is the right one
			direct_change_resource_state(dx12_commandBuffer, dx12_depthTexture->texture.resource, dx12_depthTexture->texture.state, D3D12_RESOURCE_STATE_DEPTH_WRITE);

			// Clear with the color
			dx12_commandBuffer->cmdList()->ClearDepthStencilView(rtvHandle, D3D12_CLEAR_FLAGS::D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, depth, stencil, 0, nullptr);
		}

		void clear_stencil_texture(CommandBuffer commandBuffer, RenderTexture stencilTexture, uint8_t stencil)
		{
			// Grab the actual structures
			DX12CommandBuffer* dx12_commandBuffer = (DX12CommandBuffer*)commandBuffer;
			DX12RenderTexture* dx12_stencilTexture = (DX12RenderTexture*)stencilTexture;

			// Grab the render target view
			D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle(dx12_stencilTexture->descriptorHeap->GetCPUDescriptorHandleForHeapStart());
			rtvHandle.ptr += dx12_stencilTexture->heapOffset;

			// Make sure the state is the right one
			direct_change_resource_state(dx12_commandBuffer, dx12_stencilTexture->texture.resource, dx12_stencilTexture->texture.state, D3D12_RESOURCE_STATE_DEPTH_WRITE);

			// Clear with the color
			dx12_commandBuffer->cmdList()->ClearDepthStencilView(rtvHandle, D3D12_CLEAR_FLAGS::D3D12_CLEAR_FLAG_STENCIL, 1.0, stencil, 0, nullptr);
		}

		void copy_graphics_buffer(CommandBuffer commandBuffer, GraphicsBuffer inputBuffer, GraphicsBuffer outputBuffer)
		{
			// Get the internal command buffer structure
			DX12CommandBuffer* dx12_commandBuffer = (DX12CommandBuffer*)commandBuffer;

			// Prepare the input buffer if needed
			DX12GraphicsBuffer* dx12_inputBuffer = (DX12GraphicsBuffer*)inputBuffer;
			DX12GraphicsBuffer* dx12_outputBuffer = (DX12GraphicsBuffer*)outputBuffer;

			// Change both states
			direct_change_resource_state(dx12_commandBuffer, dx12_inputBuffer->resource, dx12_inputBuffer->state, dx12_inputBuffer->heapType == GraphicsBufferType::Upload ? D3D12_RESOURCE_STATE_GENERIC_READ : D3D12_RESOURCE_STATE_COPY_SOURCE,
				dx12_outputBuffer->resource, dx12_outputBuffer->state, D3D12_RESOURCE_STATE_COPY_DEST);

			// Copy the resource
			dx12_commandBuffer->cmdList()->CopyResource(dx12_outputBuffer->resource, dx12_inputBuffer->resource);
		}

		void copy_graphics_buffer(CommandBuffer commandBuffer, GraphicsBuffer inputBuffer, uint32_t inputOffset, GraphicsBuffer outputBuffer, uint32_t outputOffset, uint32_t size)
		{
			// Get the internal command buffer structure
			DX12CommandBuffer* dx12_commandBuffer = (DX12CommandBuffer*)commandBuffer;

			// Prepare the input buffer if needed
			DX12GraphicsBuffer* dx12_inputBuffer = (DX12GraphicsBuffer*)inputBuffer;
			DX12GraphicsBuffer* dx12_outputBuffer = (DX12GraphicsBuffer*)outputBuffer;
			direct_change_resource_state(dx12_commandBuffer, dx12_inputBuffer->resource, dx12_inputBuffer->state, dx12_inputBuffer->heapType == GraphicsBufferType::Upload ? D3D12_RESOURCE_STATE_GENERIC_READ : D3D12_RESOURCE_STATE_COPY_SOURCE,
				dx12_outputBuffer->resource, dx12_outputBuffer->state, D3D12_RESOURCE_STATE_COPY_DEST);

			// Copy the resource
			dx12_commandBuffer->cmdList()->CopyBufferRegion(dx12_outputBuffer->resource, outputOffset, dx12_inputBuffer->resource, inputOffset, size);
		}

		void upload_constant_buffer(CommandBuffer commandBuffer, ConstantBuffer inputBuffer, ConstantBuffer outputBuffer)
		{
			// Get the internal command buffer structure
			DX12CommandBuffer* dx12_commandBuffer = (DX12CommandBuffer*)commandBuffer;

			// Prepare the input buffer if needed
			DX12ConstantBuffer* dx12_inputCB = (DX12ConstantBuffer*)inputBuffer;
			DX12ConstantBuffer* dx12_outputCB = (DX12ConstantBuffer*)outputBuffer;
			direct_change_resource_state(dx12_commandBuffer, dx12_inputCB->mainBuffer->resource, dx12_inputCB->mainBuffer->state, D3D12_RESOURCE_STATE_GENERIC_READ,
				dx12_outputCB->mainBuffer->resource, dx12_outputCB->mainBuffer->state, D3D12_RESOURCE_STATE_COPY_DEST);

			// Copy the resource
			dx12_commandBuffer->cmdList()->CopyResource(dx12_outputCB->mainBuffer->resource, dx12_inputCB->mainBuffer->resource);
		}

		void upload_constant_buffer(CommandBuffer commandBuffer, ConstantBuffer constantBuffer)
		{
			// Get the internal command buffer structure
			DX12CommandBuffer* dx12_commandBuffer = (DX12CommandBuffer*)commandBuffer;
			DX12ConstantBuffer* dx12_constantBuffer = (DX12ConstantBuffer*)constantBuffer;
			assert(dx12_constantBuffer->type == ConstantBufferType::Mixed);

			// Prepare the input buffer if needed
			direct_change_resource_state(dx12_commandBuffer, dx12_constantBuffer->intermediateBuffer()->resource, dx12_constantBuffer->intermediateBuffer()->state, D3D12_RESOURCE_STATE_GENERIC_READ,
				dx12_constantBuffer->mainBuffer->resource, dx12_constantBuffer->mainBuffer->state, D3D12_RESOURCE_STATE_COPY_DEST);

			// Copy the resource
			dx12_commandBuffer->cmdList()->CopyResource(dx12_constantBuffer->mainBuffer->resource, dx12_constantBuffer->intermediateBuffer()->resource);
			
			// Move on to the next intermediate buffer
			dx12_constantBuffer->instanceIdx = (dx12_constantBuffer->instanceIdx + 1) % DX12_NUM_FRAMES;
		}

		void copy_texture(CommandBuffer commandBuffer, Texture inputTexture, Texture outputTexture)
		{
			copy_texture(commandBuffer, inputTexture, 0, outputTexture, 0);
		}

		void copy_texture(CommandBuffer commandBuffer, RenderTexture inputTexture, uint32_t inputIdx, RenderTexture outputTexture, uint32_t outputIdx)
		{
			// Get the internal command buffer structure
			DX12CommandBuffer* dx12_commandBuffer = (DX12CommandBuffer*)commandBuffer;
			DX12Texture* dx12_inputTex = (DX12Texture*)inputTexture;
			DX12Texture* dx12_outputTex = (DX12Texture*)outputTexture;

			// Prepare the input buffer if needed
			direct_change_resource_state(dx12_commandBuffer, dx12_inputTex->resource, dx12_inputTex->state, D3D12_RESOURCE_STATE_COPY_SOURCE,
				dx12_outputTex->resource, dx12_outputTex->state, D3D12_RESOURCE_STATE_COPY_DEST);

			D3D12_TEXTURE_COPY_LOCATION inputResourceLoc;
			inputResourceLoc.pResource = dx12_inputTex->resource;
			inputResourceLoc.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
			inputResourceLoc.SubresourceIndex = inputIdx;

			// Prepare the output buffer if needed
			D3D12_TEXTURE_COPY_LOCATION outputResourceLoc;
			outputResourceLoc.pResource = dx12_outputTex->resource;
			outputResourceLoc.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
			outputResourceLoc.SubresourceIndex = outputIdx;

			// Box region
			D3D12_BOX box;
			box.left = 0;
			box.top = 0;
			box.front = 0;
			box.back = 1;
			if (dx12_inputTex->mipLevels == 1)
			{
				box.right = dx12_inputTex->width;
				box.bottom = dx12_inputTex->height;
			}
			else
			{
				uint32_t mipIdx = inputIdx % dx12_inputTex->mipLevels;
				box.right = dx12_inputTex->width >> mipIdx;
				box.bottom = dx12_inputTex->height >> mipIdx;
			}

			// Copy the resource
			dx12_commandBuffer->cmdList()->CopyTextureRegion(&outputResourceLoc, 0, 0, 0, &inputResourceLoc, &box);
		}

		void copy_render_texture(CommandBuffer commandBuffer, RenderTexture inputTexture, RenderTexture outputTexture)
		{
			// Get the internal command buffer structure
			DX12CommandBuffer* dx12_commandBuffer = (DX12CommandBuffer*)commandBuffer;
			DX12RenderTexture* dx12_inputTex = (DX12RenderTexture*)inputTexture;
			DX12RenderTexture* dx12_outputTex = (DX12RenderTexture*)outputTexture;

			// Run the barriers
			direct_change_resource_state(dx12_commandBuffer, dx12_inputTex->texture.resource, dx12_inputTex->texture.state, D3D12_RESOURCE_STATE_COPY_SOURCE,
				dx12_outputTex->texture.resource, dx12_outputTex->texture.state, D3D12_RESOURCE_STATE_COPY_DEST);

			D3D12_TEXTURE_COPY_LOCATION inputResourceLoc;
			inputResourceLoc.pResource = dx12_inputTex->texture.resource;
			inputResourceLoc.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
			inputResourceLoc.SubresourceIndex = 0;
			D3D12_BOX box;
			box.left = 0;
			box.right = dx12_inputTex->texture.width;
			box.top = 0;
			box.bottom = dx12_inputTex->texture.height;
			box.front = 0;
			box.back = 1;

			// Prepare the output buffer if needed
			D3D12_TEXTURE_COPY_LOCATION outputResourceLoc;
			outputResourceLoc.pResource = dx12_outputTex->texture.resource;
			outputResourceLoc.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
			outputResourceLoc.SubresourceIndex = 0;

			// Copy the resource
			dx12_commandBuffer->cmdList()->CopyTextureRegion(&outputResourceLoc, 0, 0, 0, &inputResourceLoc, &box);
		}

		void copy_buffer_into_texture(CommandBuffer commandBuffer, GraphicsBuffer inputBuffer, uint32_t bufferOffset, Texture outputTexture, uint32_t sliceIdx)
		{
			// Get the internal command buffer structure
			DX12CommandBuffer* dx12_commandBuffer = (DX12CommandBuffer*)commandBuffer;
			DX12GraphicsBuffer* dx12_inputBuffer = (DX12GraphicsBuffer*)inputBuffer;
			DX12Texture* dx12_outputTex = (DX12Texture*)outputTexture;

			// Prepare the input buffer if needed
			direct_change_resource_state(dx12_commandBuffer, dx12_inputBuffer->resource, dx12_inputBuffer->state, dx12_inputBuffer->heapType == GraphicsBufferType::Upload ? D3D12_RESOURCE_STATE_GENERIC_READ : D3D12_RESOURCE_STATE_COPY_SOURCE,
				dx12_outputTex->resource, dx12_outputTex->state, D3D12_RESOURCE_STATE_COPY_DEST);
			D3D12_TEXTURE_COPY_LOCATION inputResourceLoc;
			inputResourceLoc.pResource = dx12_inputBuffer->resource;
			inputResourceLoc.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
			inputResourceLoc.PlacedFootprint.Footprint.Format = dx12_outputTex->format;
			inputResourceLoc.PlacedFootprint.Footprint.RowPitch = dx12_outputTex->width * dx12_outputTex->alignment;
			inputResourceLoc.PlacedFootprint.Footprint.Width = dx12_outputTex->width;
			inputResourceLoc.PlacedFootprint.Footprint.Height = dx12_outputTex->height;
			inputResourceLoc.PlacedFootprint.Footprint.Depth = 1;
			inputResourceLoc.PlacedFootprint.Offset = bufferOffset;

			// Prepare the output buffer if needed
			D3D12_TEXTURE_COPY_LOCATION outputResourceLoc;
			outputResourceLoc.pResource = dx12_outputTex->resource;
			outputResourceLoc.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
			outputResourceLoc.SubresourceIndex = sliceIdx;

			D3D12_BOX box;
			box.left = 0;
			box.right = dx12_outputTex->width;
			box.top = 0;
			box.bottom = dx12_outputTex->height;
			box.front = 0;
			box.back = 1;

			// Copy the resource
			dx12_commandBuffer->cmdList()->CopyTextureRegion(&outputResourceLoc, 0, 0, 0, &inputResourceLoc, &box);
		}

		void copy_buffer_into_texture_mip(CommandBuffer commandBuffer, GraphicsBuffer inputBuffer, uint32_t bufferOffset, Texture outputTexture, uint32_t mipIdx)
		{
			// Get the internal command buffer structure
			DX12CommandBuffer* dx12_commandBuffer = (DX12CommandBuffer*)commandBuffer;
			DX12GraphicsBuffer* dx12_inputBuffer = (DX12GraphicsBuffer*)inputBuffer;
			DX12Texture* dx12_outputTex = (DX12Texture*)outputTexture;

			// Prepare the input buffer if needed
			direct_change_resource_state(dx12_commandBuffer, dx12_inputBuffer->resource, dx12_inputBuffer->state, dx12_inputBuffer->heapType == GraphicsBufferType::Upload ? D3D12_RESOURCE_STATE_GENERIC_READ : D3D12_RESOURCE_STATE_COPY_SOURCE, dx12_outputTex->resource, dx12_outputTex->state, D3D12_RESOURCE_STATE_COPY_DEST);
			
			// Dimensions of the target MIP
			uint32_t actualwidth = std::max(dx12_outputTex->width >> mipIdx, 1u);
			uint32_t actualheight = std::max(dx12_outputTex->width >> mipIdx, 1u);

			// Input buffer
			D3D12_TEXTURE_COPY_LOCATION inputResourceLoc;
			inputResourceLoc.pResource = dx12_inputBuffer->resource;
			inputResourceLoc.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
			inputResourceLoc.PlacedFootprint.Footprint.Format = dx12_outputTex->format;
			inputResourceLoc.PlacedFootprint.Footprint.RowPitch = actualwidth * dx12_outputTex->alignment;
			inputResourceLoc.PlacedFootprint.Footprint.Width = actualwidth;
			inputResourceLoc.PlacedFootprint.Footprint.Height = actualheight;
			inputResourceLoc.PlacedFootprint.Footprint.Depth = 1;
			inputResourceLoc.PlacedFootprint.Offset = bufferOffset;

			// Prepare the output buffer if needed
			D3D12_TEXTURE_COPY_LOCATION outputResourceLoc;
			outputResourceLoc.pResource = dx12_outputTex->resource;
			outputResourceLoc.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
			outputResourceLoc.SubresourceIndex = mipIdx;

			D3D12_BOX box;
			box.left = 0;
			box.right = actualwidth;
			box.top = 0;
			box.bottom = actualheight;
			box.front = 0;
			box.back = 1;

			// Copy the resource
			dx12_commandBuffer->cmdList()->CopyTextureRegion(&outputResourceLoc, 0, 0, 0, &inputResourceLoc, &box);
		}

		void copy_texture_into_buffer(CommandBuffer commandBuffer, Texture inputTexture, uint32_t sliceIdx, GraphicsBuffer outputBuffer, uint32_t bufferOffset)
		{
			// Get the internal command buffer structure
			DX12CommandBuffer* dx12_commandBuffer = (DX12CommandBuffer*)commandBuffer;
			DX12Texture* dx12_inputTex = (DX12Texture*)inputTexture;
			DX12GraphicsBuffer* dx12_outputBuffer = (DX12GraphicsBuffer*)outputBuffer;

			// Prepare the input texture if needed
			direct_change_resource_state(dx12_commandBuffer, dx12_inputTex->resource, dx12_inputTex->state, D3D12_RESOURCE_STATE_COPY_SOURCE,
				dx12_outputBuffer->resource, dx12_outputBuffer->state, D3D12_RESOURCE_STATE_COPY_DEST);

			D3D12_TEXTURE_COPY_LOCATION inputResourceLoc;
			inputResourceLoc.pResource = dx12_inputTex->resource;
			inputResourceLoc.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
			inputResourceLoc.SubresourceIndex = sliceIdx;

			// Prepare the input buffer if needed
			D3D12_TEXTURE_COPY_LOCATION outputResourceLoc;
			outputResourceLoc.pResource = dx12_outputBuffer->resource;
			outputResourceLoc.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
			outputResourceLoc.PlacedFootprint.Footprint.Format = dx12_inputTex->format;
			outputResourceLoc.PlacedFootprint.Footprint.RowPitch = dx12_inputTex->width * dx12_inputTex->alignment;
			outputResourceLoc.PlacedFootprint.Footprint.Width = dx12_inputTex->width;
			outputResourceLoc.PlacedFootprint.Footprint.Height = dx12_inputTex->height;
			outputResourceLoc.PlacedFootprint.Footprint.Depth = 1;
			outputResourceLoc.PlacedFootprint.Offset = bufferOffset;

			D3D12_BOX box;
			box.left = 0;
			box.right = dx12_inputTex->width;
			box.top = 0;
			box.bottom = dx12_inputTex->height;
			box.front = 0;
			box.back = 1;

			// Copy the resource
			dx12_commandBuffer->cmdList()->CopyTextureRegion(&outputResourceLoc, 0, 0, 0, &inputResourceLoc, &box);
		}

		void copy_render_texture_into_buffer(CommandBuffer commandBuffer, Texture inputTexture, uint32_t sliceIdx, GraphicsBuffer outputBuffer, uint32_t bufferOffset)
		{
			// Get the internal command buffer structure
			DX12RenderTexture* dx12_inputTex = (DX12RenderTexture*)inputTexture;
			copy_texture_into_buffer(commandBuffer, (Texture)&dx12_inputTex->texture, sliceIdx, outputBuffer, bufferOffset);
		}

		void copy_buffer_into_render_texture(CommandBuffer commandBuffer, GraphicsBuffer inputBuffer, uint32_t bufferOffset, Texture outputRenderTexture, uint32_t sliceIdx)
		{
			// Get the internal command buffer structure
			DX12RenderTexture* dx12_outputRT = (DX12RenderTexture*)outputRenderTexture;
			copy_buffer_into_texture(commandBuffer, inputBuffer, bufferOffset, (Texture)&dx12_outputRT->texture, sliceIdx);
		}

		void set_compute_shader_buffer_uav(CommandBuffer commandBuffer, ComputeShader computeShader, uint32_t slot, GraphicsBuffer graphicsBuffer)
		{
			if (graphicsBuffer == 0)
				return;

			// Grab all the internal structures
			DX12CommandBuffer* dx12_commandBuffer = (DX12CommandBuffer*)commandBuffer;
			DX12GraphicsDevice* deviceI = dx12_commandBuffer->deviceI;
			DX12ComputeShader* dx12_cs = (DX12ComputeShader*)computeShader;
			DX12GraphicsBuffer* buffer = (DX12GraphicsBuffer*)graphicsBuffer;

			// First we need to validate that the right heap will be used
			validate_compute_shader_heap(dx12_cs, dx12_commandBuffer->frameIdx);

			// Create the view in the compute's heap
			D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc;
			ZeroMemory(&uavDesc, sizeof(D3D12_UNORDERED_ACCESS_VIEW_DESC));
			uavDesc.Format = DXGI_FORMAT_UNKNOWN;
			uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
			D3D12_BUFFER_UAV bufferUAV;
			bufferUAV.FirstElement = 0;
			bufferUAV.NumElements = (uint32_t)buffer->bufferSize / (uint32_t)buffer->elementSize;
			bufferUAV.StructureByteStride = buffer->elementSize;
			bufferUAV.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
			bufferUAV.CounterOffsetInBytes = 0;
			uavDesc.Buffer = bufferUAV;

			// Compute the slot on the heap
			DX12DescriptorHeap& currentHeap = dx12_cs->CSUHeaps[dx12_cs->nextUsableHeap];
			D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle(currentHeap.uavCPU);
			rtvHandle.ptr += (uint64_t)deviceI->descriptorSize[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV] * slot;

			// Create the UAV
			deviceI->device->CreateUnorderedAccessView(buffer->resource, nullptr, &uavDesc, rtvHandle);

			// Change the resource's state
			async_change_resource_state(dx12_cs->barriersData, buffer->resource, buffer->state, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		}

		void set_compute_shader_texture_uav(CommandBuffer commandBuffer, ComputeShader computeShader, uint32_t slot, Texture texture, uint32_t mipLevel)
		{
			if (texture == 0)
				return;

			// Grab all the internal structures
			DX12CommandBuffer* dx12_commandBuffer = (DX12CommandBuffer*)commandBuffer;
			DX12GraphicsDevice* deviceI = dx12_commandBuffer->deviceI;
			DX12ComputeShader* dx12_cs = (DX12ComputeShader*)computeShader;
			DX12Texture* dx12_tex = (DX12Texture*)texture;

			// First we need to validate that the right heap will be used
			validate_compute_shader_heap(dx12_cs, dx12_commandBuffer->frameIdx);

			// Create the view in the compute's heap
			D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc;
			ZeroMemory(&uavDesc, sizeof(D3D12_UNORDERED_ACCESS_VIEW_DESC));
			uavDesc.Format = dx12_tex->format;

			switch (dx12_tex->type)
			{
			case TextureType::Tex2D:
			{
				D3D12_TEX2D_UAV tex2DAUAV;
				tex2DAUAV.MipSlice = mipLevel;
				tex2DAUAV.PlaneSlice = 0;
				uavDesc.Texture2D = tex2DAUAV;
				uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
			}
			break;
			case TextureType::Tex2DArray:
			{
				D3D12_TEX2D_ARRAY_UAV tex2DArrayUAV;
				tex2DArrayUAV.MipSlice = mipLevel;
				tex2DArrayUAV.FirstArraySlice = 0;
				tex2DArrayUAV.ArraySize = dx12_tex->depth;
				tex2DArrayUAV.PlaneSlice = 0;
				uavDesc.Texture2DArray = tex2DArrayUAV;
				uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
			}
			break;
			default:
				assert_fail();
			}

			// Compute the slot on the heap
			DX12DescriptorHeap& currentHeap = dx12_cs->CSUHeaps[dx12_cs->nextUsableHeap];
			D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle(currentHeap.uavCPU);
			rtvHandle.ptr += (uint64_t)deviceI->descriptorSize[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV] * slot;

			// Create the UAV
			deviceI->device->CreateUnorderedAccessView(dx12_tex->resource, nullptr, &uavDesc, rtvHandle);

			// Change the resource's state
			async_change_resource_state(dx12_cs->barriersData, dx12_tex->resource, dx12_tex->state, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		}

		void set_compute_shader_buffer_srv(CommandBuffer commandBuffer, ComputeShader computeShader, uint32_t slot, GraphicsBuffer graphicsBuffer)
		{
			if (graphicsBuffer == 0)
				return;

			// Grab all the internal structures
			DX12CommandBuffer* dx12_commandBuffer = (DX12CommandBuffer*)commandBuffer;
			DX12GraphicsDevice* deviceI = dx12_commandBuffer->deviceI;
			DX12ComputeShader* dx12_cs = (DX12ComputeShader*)computeShader;
			DX12GraphicsBuffer* buffer = (DX12GraphicsBuffer*)graphicsBuffer;

			// First we need to validate that the right heap will be used
			validate_compute_shader_heap(dx12_cs, dx12_commandBuffer->frameIdx);

			// Create the view in the compute's heap
			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
			srvDesc.Format = DXGI_FORMAT_UNKNOWN;
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
			srvDesc.Shader4ComponentMapping = D3D12_ENCODE_SHADER_4_COMPONENT_MAPPING(D3D12_SHADER_COMPONENT_MAPPING_FROM_MEMORY_COMPONENT_0, D3D12_SHADER_COMPONENT_MAPPING_FROM_MEMORY_COMPONENT_1, D3D12_SHADER_COMPONENT_MAPPING_FROM_MEMORY_COMPONENT_2, D3D12_SHADER_COMPONENT_MAPPING_FROM_MEMORY_COMPONENT_3);
			D3D12_BUFFER_SRV bufferSRV;
			bufferSRV.FirstElement = 0;
			bufferSRV.NumElements = (uint32_t)buffer->bufferSize / (uint32_t)buffer->elementSize;
			bufferSRV.StructureByteStride = buffer->elementSize;
			bufferSRV.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
			srvDesc.Buffer = bufferSRV;

			// Compute the slot on the heap
			DX12DescriptorHeap& currentHeap = dx12_cs->CSUHeaps[dx12_cs->nextUsableHeap];
			D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle(currentHeap.srvCPU);
			rtvHandle.ptr += (uint64_t)deviceI->descriptorSize[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV] * slot;

			// Create the SRV
			deviceI->device->CreateShaderResourceView(buffer->resource, &srvDesc, rtvHandle);

			// Change the resource's state
			async_change_resource_state(dx12_cs->barriersData, buffer->resource, buffer->state, D3D12_RESOURCE_STATE_COMMON);
		}

		void set_compute_shader_rtas_srv(CommandBuffer commandBuffer, ComputeShader computeShader, uint32_t slot, TopLevelAS rtas)
		{
			if (rtas == 0)
				return;

			// Grab all the internal structures
			DX12CommandBuffer* dx12_commandBuffer = (DX12CommandBuffer*)commandBuffer;
			DX12GraphicsDevice* deviceI = dx12_commandBuffer->deviceI;
			DX12ComputeShader* dx12_cs = (DX12ComputeShader*)computeShader;
			DX12TLAS* dx12_rtas = (DX12TLAS*)rtas;

			// First we need to validate that the right heap will be used
			validate_compute_shader_heap(dx12_cs, dx12_commandBuffer->frameIdx);

			// Create the view in the compute's heap
			D3D12_RAYTRACING_ACCELERATION_STRUCTURE_SRV rtasSRV;
			rtasSRV.Location = dx12_rtas->data->resource->GetGPUVirtualAddress();
			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
			srvDesc.Format = DXGI_FORMAT_UNKNOWN;
			srvDesc.Shader4ComponentMapping = D3D12_ENCODE_SHADER_4_COMPONENT_MAPPING(D3D12_SHADER_COMPONENT_MAPPING_FROM_MEMORY_COMPONENT_0, D3D12_SHADER_COMPONENT_MAPPING_FROM_MEMORY_COMPONENT_1, D3D12_SHADER_COMPONENT_MAPPING_FROM_MEMORY_COMPONENT_2, D3D12_SHADER_COMPONENT_MAPPING_FROM_MEMORY_COMPONENT_3);
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_RAYTRACING_ACCELERATION_STRUCTURE;
			srvDesc.RaytracingAccelerationStructure = rtasSRV;

			// Compute the slot on the heap
			DX12DescriptorHeap& currentHeap = dx12_cs->CSUHeaps[dx12_cs->nextUsableHeap];
			D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle(currentHeap.srvCPU);
			rtvHandle.ptr += (uint64_t)deviceI->descriptorSize[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV] * slot;

			// Create the SRV
			deviceI->device->CreateShaderResourceView(nullptr, &srvDesc, rtvHandle);

			// Change the resource's state
			async_change_resource_state(dx12_cs->barriersData, dx12_rtas->data->resource, dx12_rtas->data->state, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE);
		}

		void set_compute_shader_texture_srv(CommandBuffer commandBuffer, ComputeShader computeShader, uint32_t slot, Texture texture)
		{
			if (texture == 0)
				return;

			// Grab all the internal structures
			DX12CommandBuffer* dx12_commandBuffer = (DX12CommandBuffer*)commandBuffer;
			DX12GraphicsDevice* deviceI = dx12_commandBuffer->deviceI;
			DX12ComputeShader* dx12_cs = (DX12ComputeShader*)computeShader;
			DX12Texture* dx12_tex = (DX12Texture*)texture;

			// First we need to validate that the right heap will be used
			validate_compute_shader_heap(dx12_cs, dx12_commandBuffer->frameIdx);

			// Create the view in the compute's heap
			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
			srvDesc.Format = sanitize_dxgi_format_srv(dx12_tex->format);
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
			srvDesc.Shader4ComponentMapping = D3D12_ENCODE_SHADER_4_COMPONENT_MAPPING(D3D12_SHADER_COMPONENT_MAPPING_FROM_MEMORY_COMPONENT_0, D3D12_SHADER_COMPONENT_MAPPING_FROM_MEMORY_COMPONENT_1, D3D12_SHADER_COMPONENT_MAPPING_FROM_MEMORY_COMPONENT_2, D3D12_SHADER_COMPONENT_MAPPING_FROM_MEMORY_COMPONENT_3);
			switch (dx12_tex->type)
			{
			case TextureType::Tex2D:
			{
				D3D12_TEX2D_SRV tex2D;
				tex2D.MostDetailedMip = 0;
				tex2D.MipLevels = dx12_tex->mipLevels;
				tex2D.PlaneSlice = 0;
				tex2D.ResourceMinLODClamp = 0;
				srvDesc.Texture2D = tex2D;
				srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			}
			break;
			case TextureType::Tex2DArray:
			{
				D3D12_TEX2D_ARRAY_SRV tex2DArray;
				tex2DArray.MostDetailedMip = 0;
				tex2DArray.MipLevels = dx12_tex->mipLevels;
				tex2DArray.FirstArraySlice = 0;
				tex2DArray.ArraySize = dx12_tex->depth;
				tex2DArray.PlaneSlice = 0;
				tex2DArray.ResourceMinLODClamp = 0;
				srvDesc.Texture2DArray = tex2DArray;
				srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
			}
			break;
			default:
				assert_fail();
			}

			// Compute the slot on the heap
			DX12DescriptorHeap& currentHeap = dx12_cs->CSUHeaps[dx12_cs->nextUsableHeap];
			D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle(currentHeap.srvCPU);
			rtvHandle.ptr += (uint64_t)deviceI->descriptorSize[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV] * slot;

			// Create the SRV
			deviceI->device->CreateShaderResourceView(dx12_tex->resource, &srvDesc, rtvHandle);

			// Change the resource's state
			async_change_resource_state(dx12_cs->barriersData, dx12_tex->resource, dx12_tex->state, D3D12_RESOURCE_STATE_COMMON);
		}

		void set_compute_shader_render_texture_srv(CommandBuffer commandBuffer, ComputeShader computeShader, uint32_t slot, RenderTexture renderTexture)
		{
			if (renderTexture == 0)
				return;

			// Grab all the internal structures
			DX12RenderTexture* dx12_rTex = (DX12RenderTexture*)renderTexture;
			set_compute_shader_texture_srv(commandBuffer, computeShader, slot, (Texture)(&dx12_rTex->texture));
		}

		void set_compute_shader_render_texture_uav(CommandBuffer commandBuffer, ComputeShader computeShader, uint32_t slot, RenderTexture renderTexture)
		{
			if (renderTexture == 0)
				return;

			// Grab all the internal structures
			DX12RenderTexture* dx12_rTex = (DX12RenderTexture*)renderTexture;
			set_compute_shader_texture_uav(commandBuffer, computeShader, slot, (Texture)(&dx12_rTex->texture));
		}

		void set_compute_shader_buffer_cbv(CommandBuffer commandBuffer, ComputeShader computeShader, uint32_t slot, ConstantBuffer constantBuffer)
		{
			if (constantBuffer == 0)
				return;

			// Grab all the internal structures
			DX12CommandBuffer* dx12_commandBuffer = (DX12CommandBuffer*)commandBuffer;
			DX12GraphicsDevice* deviceI = dx12_commandBuffer->deviceI;
			DX12ComputeShader* dx12_cs = (DX12ComputeShader*)computeShader;
			DX12ConstantBuffer* dx12_cb = (DX12ConstantBuffer*)constantBuffer;
			DX12GraphicsBuffer* dx12_cbGB = dx12_cb->mainBuffer;

			// First we need to validate that the right heap will be used
			validate_compute_shader_heap(dx12_cs, dx12_commandBuffer->frameIdx);

			// Create the view in the compute's heap
			D3D12_CONSTANT_BUFFER_VIEW_DESC cbvView;
			cbvView.BufferLocation = dx12_cbGB->resource->GetGPUVirtualAddress();
			cbvView.SizeInBytes = (uint32_t)dx12_cbGB->bufferSize;

			// Compute the slot on the heap
			DX12DescriptorHeap& currentHeap = dx12_cs->CSUHeaps[dx12_cs->nextUsableHeap];
			D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle(currentHeap.cbvCPU);
			rtvHandle.ptr += (uint64_t)deviceI->descriptorSize[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV] * slot;

			// Create the CBV
			deviceI->device->CreateConstantBufferView(&cbvView, rtvHandle);

			// Change the resource's state (if this is a runtime constant buffer)
			if (dx12_cbGB->heapType != GraphicsBufferType::Upload)
				async_change_resource_state(dx12_cs->barriersData, dx12_cbGB->resource, dx12_cbGB->state, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
		}

		void set_compute_shader_sampler(CommandBuffer commandBuffer, ComputeShader computeShader, uint32_t slot, const SamplerDescriptor& smplDesc)
		{
			// Grab all the internal structures
			DX12CommandBuffer* dx12_commandBuffer = (DX12CommandBuffer*)commandBuffer;
			DX12GraphicsDevice* deviceI = dx12_commandBuffer->deviceI;
			DX12ComputeShader* dx12_cs = (DX12ComputeShader*)computeShader;
			ID3D12Device1* device = deviceI->device;

			// Set the sampler
			D3D12_SAMPLER_DESC samplerDescriptor;
			samplerDescriptor.Filter = filter_mode_to_dxgi_filter(smplDesc.filterMode);
			samplerDescriptor.AddressU = (D3D12_TEXTURE_ADDRESS_MODE)smplDesc.modeX;
			samplerDescriptor.AddressV = (D3D12_TEXTURE_ADDRESS_MODE)smplDesc.modeY;
			samplerDescriptor.AddressW = (D3D12_TEXTURE_ADDRESS_MODE)smplDesc.modeZ;
			samplerDescriptor.MipLODBias = 0.0f;
			samplerDescriptor.MaxAnisotropy = samplerDescriptor.Filter == D3D12_FILTER_ANISOTROPIC ? smplDesc.anisotropy : 0;
			samplerDescriptor.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
			memset(samplerDescriptor.BorderColor, 0, sizeof(float) * 4);
			samplerDescriptor.MinLOD = 0;
			samplerDescriptor.MaxLOD = 15;

			// Compute the slot on the heap
			DX12DescriptorHeap& currentHeap = dx12_cs->samplerHeaps[dx12_cs->nextUsableHeap];
			D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle(currentHeap.samplerCPU);
			rtvHandle.ptr += (uint64_t)deviceI->descriptorSize[D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER] * slot;

			// Set the sampler
			device->CreateSampler(&samplerDescriptor, rtvHandle);
		}

		void dispatch(CommandBuffer commandBuffer, ComputeShader computeShader, uint32_t sizeX, uint32_t sizeY, uint32_t sizeZ)
		{
			// Convert the opaque types
			DX12CommandBuffer* cmdI = (DX12CommandBuffer*)commandBuffer;
			DX12ComputeShader* dx12_cs = (DX12ComputeShader*)computeShader;

			// Process all the barriers that have been registered (at once)
			if (dx12_cs->barriersData.size() > 0)
				cmdI->cmdList()->ResourceBarrier((uint32_t)dx12_cs->barriersData.size(), dx12_cs->barriersData.data());
			dx12_cs->barriersData.clear();

			// First we need to validate that the right heap will be used
			validate_compute_shader_heap(dx12_cs, cmdI->frameIdx);

			// Set the pipeline
			cmdI->cmdList()->SetPipelineState(dx12_cs->pipelineStateObject);

			// Set the root Signature
			cmdI->cmdList()->SetComputeRootSignature(dx12_cs->rootSignature->rootSignature);

			// Bind the root descriptor tables
			DX12DescriptorHeap& currentHeap_cbv_srv_uav = dx12_cs->CSUHeaps[dx12_cs->nextUsableHeap];
			DX12DescriptorHeap& currentHeap_sampler = dx12_cs->samplerHeaps[dx12_cs->nextUsableHeap];
			ID3D12DescriptorHeap* ppHeaps[] = { currentHeap_cbv_srv_uav.descriptorHeap, currentHeap_sampler.descriptorHeap };
			cmdI->cmdList()->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

			// Bind the tables
			if (dx12_cs->rootSignature->srvIndex != UINT32_MAX)
				cmdI->cmdList()->SetComputeRootDescriptorTable(dx12_cs->rootSignature->srvIndex, currentHeap_cbv_srv_uav.srvGPU);
			if (dx12_cs->rootSignature->uavIndex != UINT32_MAX)
				cmdI->cmdList()->SetComputeRootDescriptorTable(dx12_cs->rootSignature->uavIndex, currentHeap_cbv_srv_uav.uavGPU);
			if (dx12_cs->rootSignature->cbvIndex != UINT32_MAX)
				cmdI->cmdList()->SetComputeRootDescriptorTable(dx12_cs->rootSignature->cbvIndex, currentHeap_cbv_srv_uav.cbvGPU);
			if (dx12_cs->rootSignature->samplerIndex != UINT32_MAX)
				cmdI->cmdList()->SetComputeRootDescriptorTable(dx12_cs->rootSignature->samplerIndex, currentHeap_sampler.samplerGPU);

			// Dispatch the currently bound shader
			cmdI->cmdList()->Dispatch(sizeX, sizeY, sizeZ);

			// This heap has been used for the current command buffer batch, we need to move to the next one
			dx12_cs->nextUsableHeap++;
		}

		struct IndirectDispatchCommand
		{
			D3D12_DISPATCH_ARGUMENTS dispatchArgs;
		};

		void dispatch_indirect(CommandBuffer commandBuffer, ComputeShader computeShader, GraphicsBuffer indirectBuffer, uint32_t offset)
		{
			// Convert the opaque types
			DX12CommandBuffer* cmdI = (DX12CommandBuffer*)commandBuffer;
			DX12ComputeShader* dx12_cs = (DX12ComputeShader*)computeShader;
			DX12GraphicsBuffer* dx12_indirectBuffer = (DX12GraphicsBuffer*)indirectBuffer;

			// Make sure the buffer has at least the minimal required size
			assert(dx12_indirectBuffer->bufferSize >= sizeof(uint32_t) * 3);

			// Make sure the resource is in the right state
			async_change_resource_state(dx12_cs->barriersData, dx12_indirectBuffer->resource, dx12_indirectBuffer->state, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);

			// Process all the barriers that have been registered (at once)
			if (dx12_cs->barriersData.size() > 0)
				cmdI->cmdList()->ResourceBarrier((uint32_t)dx12_cs->barriersData.size(), dx12_cs->barriersData.data());
			dx12_cs->barriersData.clear();

			// First we need to validate that the right heap will be used
			validate_compute_shader_heap(dx12_cs, cmdI->frameIdx);

			// Set the pipeline
			cmdI->cmdList()->SetPipelineState(dx12_cs->pipelineStateObject);

			// Set the root Signature
			cmdI->cmdList()->SetComputeRootSignature(dx12_cs->rootSignature->rootSignature);

			// Bind the root descriptor tables
			DX12DescriptorHeap& currentHeap_cbv_srv_uav = dx12_cs->CSUHeaps[dx12_cs->nextUsableHeap];
			DX12DescriptorHeap& currentHeap_sampler = dx12_cs->samplerHeaps[dx12_cs->nextUsableHeap];
			ID3D12DescriptorHeap* ppHeaps[] = { currentHeap_cbv_srv_uav.descriptorHeap, currentHeap_sampler.descriptorHeap };
			cmdI->cmdList()->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

			// Bind the tables
			if (dx12_cs->rootSignature->srvIndex != UINT32_MAX)
				cmdI->cmdList()->SetComputeRootDescriptorTable(dx12_cs->rootSignature->srvIndex, currentHeap_cbv_srv_uav.srvGPU);
			if (dx12_cs->rootSignature->uavIndex != UINT32_MAX)
				cmdI->cmdList()->SetComputeRootDescriptorTable(dx12_cs->rootSignature->uavIndex, currentHeap_cbv_srv_uav.uavGPU);
			if (dx12_cs->rootSignature->cbvIndex != UINT32_MAX)
				cmdI->cmdList()->SetComputeRootDescriptorTable(dx12_cs->rootSignature->cbvIndex, currentHeap_cbv_srv_uav.cbvGPU);
			if (dx12_cs->rootSignature->samplerIndex != UINT32_MAX)
				cmdI->cmdList()->SetComputeRootDescriptorTable(dx12_cs->rootSignature->samplerIndex, currentHeap_sampler.samplerGPU);


			// Execute the command
			cmdI->cmdList()->ExecuteIndirect(dx12_cs->commandSignature, 1, dx12_indirectBuffer->resource, offset, nullptr, 0);

			// This heap has been used for the current command buffer batch, we need to move to the next one
			dx12_cs->nextUsableHeap++;
		}

		void set_viewport(CommandBuffer commandBuffer, uint32_t offsetX, uint32_t offsetY, uint32_t width, uint32_t height)
		{
			// Cast the command buffer
			DX12CommandBuffer* cmdI = (DX12CommandBuffer*)commandBuffer;

			// Surface size
			D3D12_RECT surfaceSize;
			surfaceSize.left = offsetX;
			surfaceSize.top = offsetY;
			surfaceSize.right = offsetX + static_cast<LONG>(width);
			surfaceSize.bottom = offsetY + static_cast<LONG>(height);

			// Viewport
			D3D12_VIEWPORT viewport;
			viewport.TopLeftX = (float)offsetX;
			viewport.TopLeftY = (float)offsetY;
			viewport.Width = static_cast<float>(width);
			viewport.Height = static_cast<float>(height);
			viewport.MinDepth = 0.0;
			viewport.MaxDepth = 1.0;

			cmdI->cmdList()->RSSetViewports(1, &viewport);
			cmdI->cmdList()->RSSetScissorRects(1, &surfaceSize);
		}

		void set_graphics_pipeline_buffer_cbv(CommandBuffer commandBuffer, GraphicsPipeline graphicsPipeline, uint32_t slot, ConstantBuffer constantBuffer)
		{
			// Grab all the internal structures
			DX12CommandBuffer* dx12_commandBuffer = (DX12CommandBuffer*)commandBuffer;
			DX12GraphicsDevice* deviceI = dx12_commandBuffer->deviceI;
			DX12GraphicsPipeline* dx12_gp = (DX12GraphicsPipeline*)graphicsPipeline;
			DX12ConstantBuffer* dx12_cb = (DX12ConstantBuffer*)constantBuffer;
			DX12GraphicsBuffer* dx12_cbGB = dx12_cb->mainBuffer;

			// First we need to validate that the right heap will be used
			validate_graphics_pipeline_heap(dx12_gp, dx12_commandBuffer->frameIdx);

			// Create the view in the compute's heap
			D3D12_CONSTANT_BUFFER_VIEW_DESC cbvView;
			cbvView.BufferLocation = dx12_cbGB->resource->GetGPUVirtualAddress();
			cbvView.SizeInBytes = (uint32_t)dx12_cbGB->bufferSize;

			// Compute the slot on the heap
			DX12DescriptorHeap& currentHeap = dx12_gp->CSUHeaps[dx12_gp->nextUsableHeap];
			D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle(currentHeap.cbvCPU);
			rtvHandle.ptr += (uint64_t)deviceI->descriptorSize[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV] * slot;

			// Create the CBV
			deviceI->device->CreateConstantBufferView(&cbvView, rtvHandle);

			// Change the resource's state (if this is a runtime constant buffer)
			if (dx12_cbGB->heapType != GraphicsBufferType::Upload)
				async_change_resource_state(dx12_gp->barriersData, dx12_cbGB->resource, dx12_cbGB->state, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
		}

		void set_graphics_pipeline_buffer_srv(CommandBuffer commandBuffer, GraphicsPipeline graphicsPipeline, uint32_t slot, GraphicsBuffer graphicsBuffer, uint32_t bufferOffset)
		{
			// Grab all the internal structures
			DX12CommandBuffer* dx12_commandBuffer = (DX12CommandBuffer*)commandBuffer;
			DX12GraphicsDevice* deviceI = dx12_commandBuffer->deviceI;
			DX12GraphicsPipeline* dx12_gp = (DX12GraphicsPipeline*)graphicsPipeline;
			DX12GraphicsBuffer* buffer = (DX12GraphicsBuffer*)graphicsBuffer;

			// First we need to validate that the right heap will be used
			validate_graphics_pipeline_heap(dx12_gp, dx12_commandBuffer->frameIdx);

			// Create the view in the compute's heap
			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
			srvDesc.Format = DXGI_FORMAT_UNKNOWN;
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
			srvDesc.Shader4ComponentMapping = D3D12_ENCODE_SHADER_4_COMPONENT_MAPPING(D3D12_SHADER_COMPONENT_MAPPING_FROM_MEMORY_COMPONENT_0, D3D12_SHADER_COMPONENT_MAPPING_FROM_MEMORY_COMPONENT_1, D3D12_SHADER_COMPONENT_MAPPING_FROM_MEMORY_COMPONENT_2, D3D12_SHADER_COMPONENT_MAPPING_FROM_MEMORY_COMPONENT_3);
			D3D12_BUFFER_SRV bufferSRV;
			bufferSRV.FirstElement = bufferOffset / (uint32_t)buffer->elementSize;
			bufferSRV.NumElements = (uint32_t)buffer->bufferSize / (uint32_t)buffer->elementSize - (uint32_t)bufferSRV.FirstElement;
			bufferSRV.StructureByteStride = buffer->elementSize;
			bufferSRV.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
			srvDesc.Buffer = bufferSRV;

			// Compute the slot on the heap
			DX12DescriptorHeap& currentHeap = dx12_gp->CSUHeaps[dx12_gp->nextUsableHeap];
			D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle(currentHeap.srvCPU);
			rtvHandle.ptr += (uint64_t)deviceI->descriptorSize[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV] * slot;

			// Create the SRV
			deviceI->device->CreateShaderResourceView(buffer->resource, &srvDesc, rtvHandle);

			// Change the resource's state
			async_change_resource_state(dx12_gp->barriersData, buffer->resource, buffer->state, D3D12_RESOURCE_STATE_COMMON);
		}

		void set_graphics_pipeline_buffer_uav(CommandBuffer commandBuffer, GraphicsPipeline graphicsPipeline, uint32_t slot, GraphicsBuffer graphicsBuffer, uint32_t bufferOffset)
		{
			if (graphicsBuffer == 0)
				return;

			// Grab all the internal structures
			DX12CommandBuffer* dx12_commandBuffer = (DX12CommandBuffer*)commandBuffer;
			DX12GraphicsDevice* deviceI = dx12_commandBuffer->deviceI;
			DX12GraphicsPipeline* dx12_gp = (DX12GraphicsPipeline*)graphicsPipeline;
			DX12GraphicsBuffer* buffer = (DX12GraphicsBuffer*)graphicsBuffer;

			// First we need to validate that the right heap will be used
			validate_graphics_pipeline_heap(dx12_gp, dx12_commandBuffer->frameIdx);

			// Create the view in the compute's heap
			D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc;
			ZeroMemory(&uavDesc, sizeof(D3D12_UNORDERED_ACCESS_VIEW_DESC));
			uavDesc.Format = DXGI_FORMAT_UNKNOWN;
			uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
			D3D12_BUFFER_UAV bufferUAV;
			bufferUAV.FirstElement = bufferOffset / (uint32_t)buffer->elementSize;
			bufferUAV.NumElements = (uint32_t)buffer->bufferSize / (uint32_t)buffer->elementSize - (uint32_t)bufferUAV.FirstElement;
			bufferUAV.StructureByteStride = buffer->elementSize;
			bufferUAV.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
			bufferUAV.CounterOffsetInBytes = 0;
			uavDesc.Buffer = bufferUAV;

			// Compute the slot on the heap
			DX12DescriptorHeap& currentHeap = dx12_gp->CSUHeaps[dx12_gp->nextUsableHeap];
			D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle(currentHeap.uavCPU);
			rtvHandle.ptr += (uint64_t)deviceI->descriptorSize[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV] * slot;

			// Create the UAV
			deviceI->device->CreateUnorderedAccessView(buffer->resource, nullptr, &uavDesc, rtvHandle);

			// Change the resource's state
			async_change_resource_state(dx12_gp->barriersData, buffer->resource, buffer->state, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		}

		void set_graphics_pipeline_texture_srv(CommandBuffer commandBuffer, GraphicsPipeline graphicsPipeline, uint32_t slot, Texture texture)
		{
			if (texture == 0)
				return;

			// Grab all the internal structures
			DX12CommandBuffer* dx12_commandBuffer = (DX12CommandBuffer*)commandBuffer;
			DX12GraphicsDevice* deviceI = dx12_commandBuffer->deviceI;
			DX12GraphicsPipeline* dx12_gp = (DX12GraphicsPipeline*)graphicsPipeline;
			DX12Texture* dx12_tex = (DX12Texture*)texture;

			// First we need to validate that the right heap will be used
			validate_graphics_pipeline_heap(dx12_gp, dx12_commandBuffer->frameIdx);

			// Create the view in the compute's heap
			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
			srvDesc.Format = sanitize_dxgi_format_srv(dx12_tex->format);
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
			srvDesc.Shader4ComponentMapping = D3D12_ENCODE_SHADER_4_COMPONENT_MAPPING(D3D12_SHADER_COMPONENT_MAPPING_FROM_MEMORY_COMPONENT_0, D3D12_SHADER_COMPONENT_MAPPING_FROM_MEMORY_COMPONENT_1, D3D12_SHADER_COMPONENT_MAPPING_FROM_MEMORY_COMPONENT_2, D3D12_SHADER_COMPONENT_MAPPING_FROM_MEMORY_COMPONENT_3);
			switch (dx12_tex->type)
			{
			case TextureType::Tex2D:
			{
				D3D12_TEX2D_SRV tex2D;
				tex2D.MostDetailedMip = 0;
				tex2D.MipLevels = dx12_tex->mipLevels;
				tex2D.PlaneSlice = 0;
				tex2D.ResourceMinLODClamp = 0;
				srvDesc.Texture2D = tex2D;
				srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			}
			break;
			case TextureType::Tex2DArray:
			{
				D3D12_TEX2D_ARRAY_SRV tex2DArray;
				tex2DArray.MostDetailedMip = 0;
				tex2DArray.MipLevels = dx12_tex->mipLevels;
				tex2DArray.FirstArraySlice = 0;
				tex2DArray.ArraySize = dx12_tex->depth;
				tex2DArray.PlaneSlice = 0;
				tex2DArray.ResourceMinLODClamp = 0;
				srvDesc.Texture2DArray = tex2DArray;
				srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
			}
			break;
			case TextureType::TexCube:
			{
				D3D12_TEXCUBE_SRV texCube;
				texCube.MostDetailedMip = 0;
				texCube.MipLevels = dx12_tex->mipLevels;
				texCube.ResourceMinLODClamp = 0;
				srvDesc.TextureCube = texCube;
				srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
			}
			break;
			default:
				assert_fail();
			}

			// Compute the slot on the heap
			DX12DescriptorHeap& currentHeap = dx12_gp->CSUHeaps[dx12_gp->nextUsableHeap];
			D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle(currentHeap.srvCPU);
			rtvHandle.ptr += (uint64_t)deviceI->descriptorSize[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV] * slot;

			// Create the SRV
			deviceI->device->CreateShaderResourceView(dx12_tex->resource, &srvDesc, rtvHandle);

			// Change the resource's state
			async_change_resource_state(dx12_gp->barriersData, dx12_tex->resource, dx12_tex->state, dx12_tex->isDepth ? D3D12_RESOURCE_STATE_DEPTH_READ : D3D12_RESOURCE_STATE_COMMON);
		}

		void set_graphics_pipeline_render_texture_srv(CommandBuffer commandBuffer, GraphicsPipeline graphicsPipeline, uint32_t slot, RenderTexture renderTexture)
		{
			DX12RenderTexture* dx12_rTex = (DX12RenderTexture*)renderTexture;
			// Bind the texture
			set_graphics_pipeline_texture_srv(commandBuffer, graphicsPipeline, slot, (Texture)&dx12_rTex->texture);
		}

		void set_graphics_pipeline_sampler(CommandBuffer commandBuffer, GraphicsPipeline graphicsPipeline, uint32_t slot, const SamplerDescriptor& smplDesc)
		{
			// Grab all the internal structures
			DX12CommandBuffer* dx12_commandBuffer = (DX12CommandBuffer*)commandBuffer;
			DX12GraphicsDevice* deviceI = dx12_commandBuffer->deviceI;
			DX12GraphicsPipeline* dx12_gp = (DX12GraphicsPipeline*)graphicsPipeline;
			ID3D12Device1* device = deviceI->device;

			// Set the sampler
			D3D12_SAMPLER_DESC samplerDescriptor;
			samplerDescriptor.Filter = filter_mode_to_dxgi_filter(smplDesc.filterMode);
			samplerDescriptor.AddressU = (D3D12_TEXTURE_ADDRESS_MODE)smplDesc.modeX;
			samplerDescriptor.AddressV = (D3D12_TEXTURE_ADDRESS_MODE)smplDesc.modeY;
			samplerDescriptor.AddressW = (D3D12_TEXTURE_ADDRESS_MODE)smplDesc.modeZ;
			samplerDescriptor.MipLODBias = 0.0f;
			samplerDescriptor.MaxAnisotropy = samplerDescriptor.Filter == D3D12_FILTER_ANISOTROPIC ? smplDesc.anisotropy : 0;
			samplerDescriptor.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
			memset(samplerDescriptor.BorderColor, 0, sizeof(float) * 4);
			samplerDescriptor.MinLOD = 0;
			samplerDescriptor.MaxLOD = 15;

			// Compute the slot on the heap
			DX12DescriptorHeap& currentHeap = dx12_gp->samplerHeaps[dx12_gp->nextUsableHeap];
			D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle(currentHeap.samplerCPU);
			rtvHandle.ptr += (uint64_t)deviceI->descriptorSize[D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER] * slot;

			// Set the sampler
			device->CreateSampler(&samplerDescriptor, rtvHandle);
		}

		void draw_indexed(CommandBuffer commandBuffer, GraphicsPipeline graphicsPipeline, GraphicsBuffer vertexBuffer, GraphicsBuffer indexBuffer, uint32_t num_triangles, uint32_t numInstances, DrawPrimitive primitive)
		{
			DX12CommandBuffer* cmdI = (DX12CommandBuffer*)commandBuffer;
			DX12GraphicsPipeline* dx12_gp = (DX12GraphicsPipeline*)graphicsPipeline;

			cmdI->cmdList()->SetPipelineState(dx12_gp->pipelineStateObject);

			// Process all the barriers that have been registered (at once)
			if (dx12_gp->barriersData.size() > 0)
				cmdI->cmdList()->ResourceBarrier((uint32_t)dx12_gp->barriersData.size(), dx12_gp->barriersData.data());
			dx12_gp->barriersData.clear();

			// First we need to validate that the right heap will be used
			validate_graphics_pipeline_heap(dx12_gp, cmdI->frameIdx);

			// Set the pipeline state
			cmdI->cmdList()->SetPipelineState(dx12_gp->pipelineStateObject);

			// Set the root signature
			cmdI->cmdList()->SetGraphicsRootSignature(dx12_gp->rootSignature->rootSignature);

			// Set the descriptor heap
			DX12DescriptorHeap& currentHeap_cbv_srv_uav = dx12_gp->CSUHeaps[dx12_gp->nextUsableHeap];
			DX12DescriptorHeap& currentHeap_sampler = dx12_gp->samplerHeaps[dx12_gp->nextUsableHeap];
			ID3D12DescriptorHeap* ppHeaps[] = { currentHeap_cbv_srv_uav.descriptorHeap, currentHeap_sampler.descriptorHeap };
			cmdI->cmdList()->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

			// Set the tables
			if (dx12_gp->rootSignature->srvIndex != UINT32_MAX)
				cmdI->cmdList()->SetGraphicsRootDescriptorTable(dx12_gp->rootSignature->srvIndex, currentHeap_cbv_srv_uav.srvGPU);
			if (dx12_gp->rootSignature->uavIndex != UINT32_MAX)
				cmdI->cmdList()->SetGraphicsRootDescriptorTable(dx12_gp->rootSignature->uavIndex, currentHeap_cbv_srv_uav.uavGPU);
			if (dx12_gp->rootSignature->cbvIndex != UINT32_MAX)
				cmdI->cmdList()->SetGraphicsRootDescriptorTable(dx12_gp->rootSignature->cbvIndex, currentHeap_cbv_srv_uav.cbvGPU);
			if (dx12_gp->rootSignature->samplerIndex != UINT32_MAX)
				cmdI->cmdList()->SetGraphicsRootDescriptorTable(dx12_gp->rootSignature->samplerIndex, currentHeap_sampler.samplerGPU);

			if (primitive == DrawPrimitive::Triangle)
			{
				// Set the right primitive
				if (dx12_gp->hullblob != nullptr && dx12_gp->domainBlob != nullptr)
					cmdI->cmdList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);
				else
					cmdI->cmdList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			}
			else
			{
				// Set the right primitive
				if (dx12_gp->hullblob != nullptr && dx12_gp->domainBlob != nullptr)
					cmdI->cmdList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_2_CONTROL_POINT_PATCHLIST);
				else
					cmdI->cmdList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
			}

			// Bind the vertex buffer
			{
				DX12GraphicsBuffer* dx12_vertexBuffer = (DX12GraphicsBuffer*)vertexBuffer;
				D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
				vertexBufferView.BufferLocation = dx12_vertexBuffer->resource->GetGPUVirtualAddress();
				vertexBufferView.StrideInBytes = (uint32_t)dx12_vertexBuffer->elementSize;
				vertexBufferView.SizeInBytes = (uint32_t)dx12_vertexBuffer->bufferSize;
				cmdI->cmdList()->IASetVertexBuffers(0, 1, &vertexBufferView);
			}

			// Bind the index buffer
			{
				DX12GraphicsBuffer* dx12_indexBuffer = (DX12GraphicsBuffer*)indexBuffer;
				D3D12_INDEX_BUFFER_VIEW indexBufferView;
				indexBufferView.BufferLocation = dx12_indexBuffer->resource->GetGPUVirtualAddress();
				indexBufferView.Format = DXGI_FORMAT_R32_UINT;
				indexBufferView.SizeInBytes = (uint32_t)dx12_indexBuffer->bufferSize;
				cmdI->cmdList()->IASetIndexBuffer(&indexBufferView);
			}

			// Set the stencil ref
			cmdI->cmdList()->OMSetStencilRef(dx12_gp->stencilRef);

			// Draw the primitives
			if (primitive == DrawPrimitive::Triangle)
				cmdI->cmdList()->DrawIndexedInstanced(3 * num_triangles, numInstances, 0, 0, 0);
			else
				cmdI->cmdList()->DrawIndexedInstanced(2 * num_triangles, numInstances, 0, 0, 0);

			// This heap has been used for the current command buffer batch, we need to move to the next one
			dx12_gp->nextUsableHeap++;
		}

		void draw_procedural(CommandBuffer commandBuffer, GraphicsPipeline graphicsPipeline, uint32_t numTriangles, uint32_t numInstances, DrawPrimitive primitive)
		{
			// Convert the opaque types
			DX12CommandBuffer* cmdI = (DX12CommandBuffer*)commandBuffer;
			DX12GraphicsPipeline* dx12_gp = (DX12GraphicsPipeline*)graphicsPipeline;

			// Process all the barriers that have been registered (at once)
			if (dx12_gp->barriersData.size() > 0)
				cmdI->cmdList()->ResourceBarrier((uint32_t)dx12_gp->barriersData.size(), dx12_gp->barriersData.data());
			dx12_gp->barriersData.clear();

			// First we need to validate that the right heap will be used
			validate_graphics_pipeline_heap(dx12_gp, cmdI->frameIdx);

			// Set the pipeline state
			cmdI->cmdList()->SetPipelineState(dx12_gp->pipelineStateObject);

			// Set the root signature
			cmdI->cmdList()->SetGraphicsRootSignature(dx12_gp->rootSignature->rootSignature);

			// Set the descriptor heap
			DX12DescriptorHeap& currentHeap_cbv_srv_uav = dx12_gp->CSUHeaps[dx12_gp->nextUsableHeap];
			DX12DescriptorHeap& currentHeap_sampler = dx12_gp->samplerHeaps[dx12_gp->nextUsableHeap];
			ID3D12DescriptorHeap* ppHeaps[] = { currentHeap_cbv_srv_uav.descriptorHeap, currentHeap_sampler.descriptorHeap };
			cmdI->cmdList()->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

			// Set the tables
			if (dx12_gp->rootSignature->srvIndex != UINT32_MAX)
				cmdI->cmdList()->SetGraphicsRootDescriptorTable(dx12_gp->rootSignature->srvIndex, currentHeap_cbv_srv_uav.srvGPU);
			if (dx12_gp->rootSignature->uavIndex != UINT32_MAX)
				cmdI->cmdList()->SetGraphicsRootDescriptorTable(dx12_gp->rootSignature->uavIndex, currentHeap_cbv_srv_uav.uavGPU);
			if (dx12_gp->rootSignature->cbvIndex != UINT32_MAX)
				cmdI->cmdList()->SetGraphicsRootDescriptorTable(dx12_gp->rootSignature->cbvIndex, currentHeap_cbv_srv_uav.cbvGPU);
			if (dx12_gp->rootSignature->samplerIndex != UINT32_MAX)
				cmdI->cmdList()->SetGraphicsRootDescriptorTable(dx12_gp->rootSignature->samplerIndex, currentHeap_sampler.samplerGPU);

			// Set the right primitive
			if (dx12_gp->hullblob != nullptr && dx12_gp->domainBlob != nullptr)
				cmdI->cmdList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);
			else
				cmdI->cmdList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINESTRIP);

			if (primitive == DrawPrimitive::Triangle)
			{
				// Set the right primitive
				if (dx12_gp->hullblob != nullptr && dx12_gp->domainBlob != nullptr)
					cmdI->cmdList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);
				else
					cmdI->cmdList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			}
			else
			{
				// Set the right primitive
				if (dx12_gp->hullblob != nullptr && dx12_gp->domainBlob != nullptr)
					cmdI->cmdList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_2_CONTROL_POINT_PATCHLIST);
				else
					cmdI->cmdList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
			}

			// Set the stencil ref
			cmdI->cmdList()->OMSetStencilRef(dx12_gp->stencilRef);

			// Draw instanced
			if (primitive == DrawPrimitive::Triangle)
				cmdI->cmdList()->DrawInstanced(3 * numTriangles, numInstances, 0, 0);
			else
				cmdI->cmdList()->DrawInstanced(2 * numTriangles, numInstances, 0, 0);

			// This heap has been used for the current command buffer batch, we need to move to the next one
			dx12_gp->nextUsableHeap++;
		}

		void draw_procedural_indirect(CommandBuffer commandBuffer, GraphicsPipeline graphicsPipeline, GraphicsBuffer indirectBuffer, uint32_t bufferOffset)
		{
			// Convert the opaque types
			DX12CommandBuffer* cmdI = (DX12CommandBuffer*)commandBuffer;
			DX12GraphicsPipeline* dx12_gp = (DX12GraphicsPipeline*)graphicsPipeline;
			DX12GraphicsBuffer* dx12_indirectBuffer = (DX12GraphicsBuffer*)indirectBuffer;

			// Make sure the buffer has at least the minimal required size
			assert(dx12_indirectBuffer->bufferSize >= sizeof(uint32_t) * 4);

			// Make sure the resource is in the right state
			async_change_resource_state(dx12_gp->barriersData, dx12_indirectBuffer->resource, dx12_indirectBuffer->state, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);

			// Process all the barriers that have been registered (at once)
			if (dx12_gp->barriersData.size() > 0)
				cmdI->cmdList()->ResourceBarrier((uint32_t)dx12_gp->barriersData.size(), dx12_gp->barriersData.data());
			dx12_gp->barriersData.clear();

			// First we need to validate that the right heap will be used
			validate_graphics_pipeline_heap(dx12_gp, cmdI->frameIdx);

			// Set the pipeline state
			cmdI->cmdList()->SetPipelineState(dx12_gp->pipelineStateObject);

			// Set the root signature
			cmdI->cmdList()->SetGraphicsRootSignature(dx12_gp->rootSignature->rootSignature);

			// Set the descriptor heap
			DX12DescriptorHeap& currentHeap_cbv_srv_uav = dx12_gp->CSUHeaps[dx12_gp->nextUsableHeap];
			DX12DescriptorHeap& currentHeap_sampler = dx12_gp->samplerHeaps[dx12_gp->nextUsableHeap];
			ID3D12DescriptorHeap* ppHeaps[] = { currentHeap_cbv_srv_uav.descriptorHeap, currentHeap_sampler.descriptorHeap };
			cmdI->cmdList()->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

			// Set the tables
			if (dx12_gp->rootSignature->srvIndex != UINT32_MAX)
				cmdI->cmdList()->SetGraphicsRootDescriptorTable(dx12_gp->rootSignature->srvIndex, currentHeap_cbv_srv_uav.srvGPU);
			if (dx12_gp->rootSignature->uavIndex != UINT32_MAX)
				cmdI->cmdList()->SetGraphicsRootDescriptorTable(dx12_gp->rootSignature->uavIndex, currentHeap_cbv_srv_uav.uavGPU);
			if (dx12_gp->rootSignature->cbvIndex != UINT32_MAX)
				cmdI->cmdList()->SetGraphicsRootDescriptorTable(dx12_gp->rootSignature->cbvIndex, currentHeap_cbv_srv_uav.cbvGPU);
			if (dx12_gp->rootSignature->samplerIndex != UINT32_MAX)
				cmdI->cmdList()->SetGraphicsRootDescriptorTable(dx12_gp->rootSignature->samplerIndex, currentHeap_sampler.samplerGPU);

			// Set the right stencil
			cmdI->cmdList()->OMSetStencilRef(dx12_gp->stencilRef);

			// Set the right primitive
			if (dx12_gp->hullblob != nullptr && dx12_gp->domainBlob != nullptr)
				cmdI->cmdList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST);
			else
				cmdI->cmdList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

			// Execute the command
			cmdI->cmdList()->ExecuteIndirect(dx12_gp->commandSignature, 1, dx12_indirectBuffer->resource, bufferOffset, nullptr, 0);

			// This heap has been used for the current command buffer batch, we need to move to the next one
			dx12_gp->nextUsableHeap++;
		}

		void build_blas(CommandBuffer cmdB, BottomLevelAS blas)
		{
			DX12CommandBuffer* dx12_cmdB = (DX12CommandBuffer*)cmdB;
			DX12BLAS* dx12_blas = (DX12BLAS*)blas;

			// Describe the build
			D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC bottomLevelBuildDesc = {};
			bottomLevelBuildDesc.Inputs = dx12_blas->inputs;
			bottomLevelBuildDesc.ScratchAccelerationStructureData = dx12_blas->scratchBuffer->resource->GetGPUVirtualAddress();
			bottomLevelBuildDesc.DestAccelerationStructureData = dx12_blas->data->resource->GetGPUVirtualAddress();

			// Change the stat of the input buffers
			direct_change_resource_state(dx12_cmdB, dx12_blas->vertexBuffer->resource, dx12_blas->vertexBuffer->state, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
				dx12_blas->indexBuffer->resource, dx12_blas->indexBuffer->state, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

			// Build the blas
			dx12_cmdB->cmdList()->BuildRaytracingAccelerationStructure(&bottomLevelBuildDesc, 0, nullptr);

			// Transition to an UAV
			uav_barrier_buffer(cmdB, (GraphicsBuffer)dx12_blas->data);
		}

		void build_tlas(CommandBuffer cmdB, TopLevelAS tlas)
		{
			DX12CommandBuffer* dx12_cmdB = (DX12CommandBuffer*)cmdB;
			DX12TLAS* dx12_tlas = (DX12TLAS*)tlas;

			// Top Level Acceleration Structure desc
			D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC topLevelBuildDesc = {};
			topLevelBuildDesc.Inputs = dx12_tlas->inputs;
			topLevelBuildDesc.DestAccelerationStructureData = dx12_tlas->data->resource->GetGPUVirtualAddress();
			topLevelBuildDesc.ScratchAccelerationStructureData = dx12_tlas->scratchBuffer->resource->GetGPUVirtualAddress();

			// Build the tlas
			dx12_cmdB->cmdList()->BuildRaytracingAccelerationStructure(&topLevelBuildDesc, 0, nullptr);
			uav_barrier_buffer(cmdB, (GraphicsBuffer)dx12_tlas->data);
		}

		void start_section(CommandBuffer commandBuffer, const std::string& eventName)
		{
			DX12CommandBuffer* cmdI = (DX12CommandBuffer*)commandBuffer;
			PIXBeginEvent(cmdI->cmdList(), 0, eventName.c_str());
		}

		void end_section(CommandBuffer commandBuffer)
		{
			DX12CommandBuffer* cmdI = (DX12CommandBuffer*)commandBuffer;
			PIXEndEvent(cmdI->cmdList());
		}

		void enable_profiling_scope(CommandBuffer commandBuffer, ProfilingScope profilingScope)
		{
			DX12CommandBuffer* cmdI = (DX12CommandBuffer*)commandBuffer;
			DX12Query* query = (DX12Query*)profilingScope;
			cmdI->cmdList()->EndQuery(query->heap, D3D12_QUERY_TYPE_TIMESTAMP, 0);
		}

		void disable_profiling_scope(CommandBuffer commandBuffer, ProfilingScope profilingScope)
		{
			DX12CommandBuffer* cmdI = (DX12CommandBuffer*)commandBuffer;
			DX12Query* query = (DX12Query*)profilingScope;
			cmdI->cmdList()->EndQuery(query->heap, D3D12_QUERY_TYPE_TIMESTAMP, 1);
			// Resolve the occlusion query and store the results in the query result buffer to be used on the subsequent frame.
			cmdI->cmdList()->ResolveQueryData(query->heap, D3D12_QUERY_TYPE_TIMESTAMP, 0, 2, query->result, 0);
		}
	}
}

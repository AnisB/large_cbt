// Internal includes
#include "graphics/dx12_backend.h"
#include "graphics/dx12_containers.h"
#include "graphics/dx12_helpers.h"
#include "tools/string_utilities.h"
#include "tools/security.h"

namespace d3d12
{
	// Function to create the swap chain
	IDXGISwapChain4* CreateSwapChain(HWND hWnd, ID3D12CommandQueue* commandQueue, uint32_t width, uint32_t height, uint32_t bufferCount, DXGI_FORMAT format)
	{
		// Grab the DXGI factory 2
		IDXGIFactory4* dxgiFactory4;
		UINT createFactoryFlags = 0;
		assert_msg(CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&dxgiFactory4)) == S_OK, "DXGI Factory 2 request failed.");

		// Describe the swap chain
		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
		swapChainDesc.Width = width;
		swapChainDesc.Height = height;
		swapChainDesc.Format = format;
		swapChainDesc.Stereo = FALSE;
		swapChainDesc.SampleDesc = { 1, 0 };
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = bufferCount;
		swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
		swapChainDesc.Flags = 0;

		// Create the swapchain
		IDXGISwapChain1* swapChain1;
		assert_msg(dxgiFactory4->CreateSwapChainForHwnd(commandQueue, hWnd, &swapChainDesc, nullptr, nullptr, &swapChain1) == S_OK, "Create Swap Chain failed.");

		// Cnonvert to the Swap Chain 4 structure
		IDXGISwapChain4* dxgiSwapChain4 = (IDXGISwapChain4*)swapChain1;

		// Release the resources
		dxgiFactory4->Release();

		// Return the swap chain
		return dxgiSwapChain4;
	}

	// Swap Chain API
	namespace swap_chain
	{
		// Creation and Destruction
		SwapChain create_swap_chain(RenderWindow renderWindow, GraphicsDevice graphicsDevice, CommandQueue commandQueue, TextureFormat format)
		{
			// Grab the actual structures
			DX12Window* dx12_window = (DX12Window*)renderWindow;
			DX12CommandQueue* dx12_commandQueue = (DX12CommandQueue*)commandQueue;
			DX12GraphicsDevice* deviceI = (DX12GraphicsDevice*)graphicsDevice;
			ID3D12Device1* device = deviceI->device;

			// Create the render environment internal structure
			DX12SwapChain* swapChainI = new DX12SwapChain();

			// Get the dx12 format
			DXGI_FORMAT dxgi_format = format_to_dxgi_format(format);

			// Create the swap chain
			uint2 viewportSize;
			window::viewport_size(renderWindow, viewportSize);
			swapChainI->swapChain = CreateSwapChain(dx12_window->window, dx12_commandQueue->directSubQueue.queue, viewportSize.x, viewportSize.y, DX12_NUM_FRAMES, dxgi_format);

			// Grab the current back buffer
			swapChainI->currentBackBuffer = swapChainI->swapChain->GetCurrentBackBufferIndex();

			// Create the descriptor heap for the swap chain
			swapChainI->descriptorHeap = create_descriptor_heap_internal(deviceI, DX12_NUM_FRAMES, (uint32_t)D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

			// Start of the heap
			D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle(swapChainI->descriptorHeap->GetCPUDescriptorHandleForHeapStart());

			// Create a RTV for each frame.
			for (uint32_t n = 0; n < DX12_NUM_FRAMES; n++)
			{
				// Keep track of the descriptor heap where this is stored
				DX12RenderTexture& currentTexture = swapChainI->backBufferRenderTextures[n];
				currentTexture.texture.width = viewportSize.x;
				currentTexture.texture.height = viewportSize.y;
				currentTexture.texture.depth = 1;
				currentTexture.texture.format = dxgi_format;
				currentTexture.texture.state = D3D12_RESOURCE_STATE_PRESENT;
				currentTexture.texture.type = TextureType::Tex2D;
				currentTexture.descriptorHeap = swapChainI->descriptorHeap;
				currentTexture.heapOffset = deviceI->descriptorSize[D3D12_DESCRIPTOR_HEAP_TYPE_RTV] * n;

				// Grab the buffer of the swap chain
				assert_msg(swapChainI->swapChain->GetBuffer(n, IID_PPV_ARGS(&swapChainI->backBufferRenderTextures[n].texture.resource)) == S_OK, "Failed to get the swap chain buffer.");

				// Create a render target view for it
				device->CreateRenderTargetView(swapChainI->backBufferRenderTextures[n].texture.resource, nullptr, rtvHandle);

				// Move on to the next pointer
				rtvHandle.ptr += (1 * deviceI->descriptorSize[D3D12_DESCRIPTOR_HEAP_TYPE_RTV]);
			}

			// Return the opaque structure
			return (SwapChain)swapChainI;
		}

		void destroy_swap_chain(SwapChain swapChain)
		{
			// Grab the actual structure
			DX12SwapChain* dx12_swapChain = (DX12SwapChain*)swapChain;

			// Release the render target views
			for (uint32_t n = 0; n < DX12_NUM_FRAMES; n++)
				dx12_swapChain->backBufferRenderTextures[n].texture.resource->Release();

			// Release the DX12 structures
			dx12_swapChain->descriptorHeap->Release();
			dx12_swapChain->swapChain->Release();

			// Release the internal structure
			delete dx12_swapChain;
		}

		RenderTexture get_current_render_texture(SwapChain swapChain)
		{
			DX12SwapChain* dx12_swapChain = (DX12SwapChain*)swapChain;
			return (RenderTexture)(&dx12_swapChain->backBufferRenderTextures[dx12_swapChain->currentBackBuffer]);
		}

		void present(SwapChain swapChain)
		{
			// Convert to the internal structure
			DX12SwapChain* dx12_swapChain = (DX12SwapChain*)swapChain;

			// Present the frame buffer
			assert_msg(dx12_swapChain->swapChain->Present(0, 0) == S_OK, "Swap Chain Present failed.");

			// Update the current back buffer
			dx12_swapChain->currentBackBuffer = dx12_swapChain->swapChain->GetCurrentBackBufferIndex();
		}
	}
}

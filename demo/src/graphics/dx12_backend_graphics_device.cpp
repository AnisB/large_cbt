// Internal includes
#include "graphics/dx12_backend.h"
#include "graphics/dx12_containers.h"
#include "graphics/dx12_helpers.h"
#include "tools/security.h"
#include "tools/string_utilities.h"

// System includes
#include <algorithm>
#include <initguid.h>
#include "dxgidebug.h"

// Export if we are running in experimental
extern "C" { __declspec(dllexport) extern const UINT D3D12SDKVersion = 614; }
extern "C" { __declspec(dllexport) extern const char* D3D12SDKPath = u8".\\D3D12\\"; }

// Tracking if the debug layer was enabled
static bool g_debugLayerEnabled = false;

namespace d3d12
{
    namespace graphics_device
    {
        IDXGIAdapter1* GetLargestAdapter()
        {
            // Create the DXGI factory
            IDXGIFactory4* dxgiFactory;
            UINT createFactoryFlags = 0;
            assert_msg(CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&dxgiFactory)) == S_OK, "DXGI Factory 2 failed.");

            // The final adapter that we will be using
            IDXGIAdapter1* targetAdapter = nullptr;

            // Loop through all the available dapters
            IDXGIAdapter1* dxgiAdapter1;
            SIZE_T maxDedicatedVideoMemory = 0;

            // Loop through the adapters
            for (uint32_t adapterIdx = 0; dxgiFactory->EnumAdapters1(adapterIdx, &dxgiAdapter1) != DXGI_ERROR_NOT_FOUND; ++adapterIdx)
            {
                // Grab the descriptor of this adapter
                DXGI_ADAPTER_DESC1 dxgiAdapterDesc1;
                dxgiAdapter1->GetDesc1(&dxgiAdapterDesc1);

                // First is it usable?
                bool usableGPU = (dxgiAdapterDesc1.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) == 0 && SUCCEEDED(D3D12CreateDevice(dxgiAdapter1, D3D_FEATURE_LEVEL_12_2, __uuidof(ID3D12Device), nullptr));

                // Is it bigger?
                if (dxgiAdapterDesc1.DedicatedVideoMemory >= maxDedicatedVideoMemory)
                {
                    // New size
                    maxDedicatedVideoMemory = dxgiAdapterDesc1.DedicatedVideoMemory;

                    // Keep track of the adapter
                    targetAdapter = dxgiAdapter1;
                }
            }

            // Release the factory
            dxgiFactory->Release();

            // Return the adapter
            return targetAdapter;
        }

        IDXGIAdapter1* GetAdapterGPUVendor(GPUVendor vendor)
        {
            // Get the vendor ID
            uint32_t vendorID = vendor_to_vendor_id(vendor);

            // Create the DXGI factory
            IDXGIFactory4* dxgiFactory;
            UINT createFactoryFlags = 0;
            assert_msg(CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&dxgiFactory)) == S_OK, "DXGI Factory 2 failed.");

            // The final adapter that we will be using
            IDXGIAdapter1* targetAdapter = nullptr;

            // Loop through all the available dapters
            IDXGIAdapter1* dxgiAdapter1;
            SIZE_T maxDedicatedVideoMemory = 0;

            // Loop through the adapters
            for (uint32_t adapterIdx = 0; dxgiFactory->EnumAdapters1(adapterIdx, &dxgiAdapter1) != DXGI_ERROR_NOT_FOUND; ++adapterIdx)
            {
                // Grab the descriptor of this adapter
                DXGI_ADAPTER_DESC1 dxgiAdapterDesc1;
                dxgiAdapter1->GetDesc1(&dxgiAdapterDesc1);

                // First is it usable?
                bool usableGPU = (dxgiAdapterDesc1.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) == 0 && SUCCEEDED(D3D12CreateDevice(dxgiAdapter1, D3D_FEATURE_LEVEL_12_2, __uuidof(ID3D12Device), nullptr));

                // Is it the right vendor?
                if (usableGPU && dxgiAdapterDesc1.VendorId == vendorID)
                {
                    targetAdapter = dxgiAdapter1;
                    break;
                }
            }

            // Release the factory
            dxgiFactory->Release();

            // Return the adapter
            return targetAdapter;
        }

        IDXGIAdapter1* GetAdapterbyID(uint32_t adapterID)
        {
            // Create the DXGI factory
            IDXGIFactory4* dxgiFactory;
            UINT createFactoryFlags = 0;
            assert_msg(CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&dxgiFactory)) == S_OK, "DXGI Factory 2 failed.");

            // The final adapter that we will be using
            IDXGIAdapter1* targetAdapter = nullptr;

            // Loop through all the available dapters
            IDXGIAdapter1* dxgiAdapter1;
            SIZE_T maxDedicatedVideoMemory = 0;

            // Loop through the adapters
            for (uint32_t adapterIdx = 0; dxgiFactory->EnumAdapters1(adapterIdx, &dxgiAdapter1) != DXGI_ERROR_NOT_FOUND; ++adapterIdx)
            {
                // Grab the descriptor of this adapter
                DXGI_ADAPTER_DESC1 dxgiAdapterDesc1;
                dxgiAdapter1->GetDesc1(&dxgiAdapterDesc1);

                // First is it usable?
                bool usableGPU = (dxgiAdapterDesc1.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) == 0 && SUCCEEDED(D3D12CreateDevice(dxgiAdapter1, D3D_FEATURE_LEVEL_12_1, __uuidof(ID3D12Device), nullptr));

                // Is it the right vendor?
                if (usableGPU && adapterIdx == adapterID)
                    targetAdapter = dxgiAdapter1;
                else
                    dxgiAdapter1->Release();
            }

            // Release the factory
            dxgiFactory->Release();

            // Return the adapter
            return targetAdapter;
        }

        void enable_debug_layer()
        {
            ID3D12Debug6* debugLayer = nullptr;
            assert_msg(D3D12GetDebugInterface(IID_PPV_ARGS(&debugLayer)) == S_OK, "Debug layer failed to initialize");
            debugLayer->EnableDebugLayer();
            g_debugLayerEnabled = true;
            debugLayer->Release();
        }

        GraphicsDevice create_graphics_device_vendorid(GPUVendor vendor)
        {
            return create_graphics_device(DevicePickStrategy::VendorID, (uint32_t)vendor);
        }

        GraphicsDevice create_graphics_device(DevicePickStrategy pickStrategy, uint32_t id)
        {
            // Create the graphics device internal structure
            DX12GraphicsDevice* dx12_device = new DX12GraphicsDevice();

            // Grab the right adapter based on the requested strategy
            IDXGIAdapter1* adapter = nullptr;
            if (pickStrategy ==  DevicePickStrategy::VRAMSize)
                adapter = GetLargestAdapter();
            else if (pickStrategy == DevicePickStrategy::VendorID)
                adapter = GetAdapterGPUVendor((GPUVendor)id);
            else if (pickStrategy == DevicePickStrategy::AdapterID)
                adapter = GetAdapterbyID(id);

            // Did we fail to find the adapter?
            if (adapter == nullptr)
            {
                printf("Failed to find the target adapter\n");
                return 0;
            }

            // Grab the descriptor of this adapter
            DXGI_ADAPTER_DESC1 dxgiAdapterDesc1;
            adapter->GetDesc1(&dxgiAdapterDesc1);
            dx12_device->adapterName = convert_to_regular(dxgiAdapterDesc1.Description);
            // wprintf(L"Picked GPU: %s\n", actualDescriptor.Description);

            // Create the graphics device and ensure it's been succesfully created
            HRESULT result = D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&dx12_device->device));
            assert_msg(result == S_OK, "D3D12 Device creation failed.");
            // Do not forget to release the adapter
            adapter->Release();

            // Keep track of the structures and data
            for (int i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i)
                dx12_device->descriptorSize[i] = dx12_device->device->GetDescriptorHandleIncrementSize((D3D12_DESCRIPTOR_HEAP_TYPE)i);

            // Get the vendor
            dx12_device->vendor = vendor_id_to_vendor(dxgiAdapterDesc1.VendorId);

            // Check the ray tracing support
            D3D12_FEATURE_DATA_D3D12_OPTIONS options = {};
            assert_msg(dx12_device->device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS, &options, sizeof(options)) == S_OK, "Failed to query option.");
            D3D12_FEATURE_DATA_D3D12_OPTIONS1 options1 = {};
            assert_msg(dx12_device->device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS1, &options1, sizeof(options1)) == S_OK, "Failed to query option1.");
            D3D12_FEATURE_DATA_D3D12_OPTIONS4 options4 = {};
            assert_msg(dx12_device->device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS4, &options4, sizeof(options4)) == S_OK, "Failed to query option4.");
            D3D12_FEATURE_DATA_D3D12_OPTIONS5 options5 = {};
            assert_msg(dx12_device->device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &options5, sizeof(options5)) == S_OK, "Failed to query option5.");
            D3D12_FEATURE_DATA_D3D12_OPTIONS6 options6 = {};
            assert_msg(dx12_device->device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS6, &options6, sizeof(options6)) == S_OK, "Failed to query option6.");
            D3D12_FEATURE_DATA_D3D12_OPTIONS7 options7 = {};
            assert_msg(dx12_device->device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS7, &options7, sizeof(options7)) == S_OK, "Failed to query option7.");
            D3D12_FEATURE_DATA_D3D12_OPTIONS8 options8 = {};
            assert_msg(dx12_device->device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS8, &options8, sizeof(options8)) == S_OK, "Failed to query option8.");
            D3D12_FEATURE_DATA_D3D12_OPTIONS9 options9 = {};
            assert_msg(dx12_device->device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS9, &options9, sizeof(options9)) == S_OK, "Failed to query option9.");
            D3D12_FEATURE_DATA_D3D12_OPTIONS10 options10 = {};
            assert_msg(dx12_device->device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS10, &options10, sizeof(options10)) == S_OK, "Failed to query option10.");

            // Keep track of the all the options
            dx12_device->supportDoubleShaderOps = options.DoublePrecisionFloatShaderOps;
            dx12_device->support16bitShaderOps = options4.Native16BitShaderOpsSupported;
            dx12_device->supportRayTracing = options5.RaytracingTier > D3D12_RAYTRACING_TIER_NOT_SUPPORTED;
            dx12_device->supportWaveMMA = options9.WaveMMATier != D3D12_WAVE_MMA_TIER_NOT_SUPPORTED;

            return (GraphicsDevice)dx12_device;
        }

        void destroy_graphics_device(GraphicsDevice graphicsDevice)
        {
            DX12GraphicsDevice* dx12_device = (DX12GraphicsDevice*)graphicsDevice;
            dx12_device->device->Release();
            delete dx12_device;
        }

        void report_live_objects()
        {
            if (!g_debugLayerEnabled)
                printf("[WARNING] Calling report live objects without enabling the debug layer.");
            IDXGIDebug1* debugIt;
            UINT debugFlags = 0;
            DXGIGetDebugInterface1(debugFlags, IID_PPV_ARGS(&debugIt));
            debugIt->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_DETAIL);
        }

        void set_stable_power_state(GraphicsDevice device, bool state)
        {
            DX12GraphicsDevice* dx12_device = (DX12GraphicsDevice*)device;
            dx12_device->device->SetStablePowerState(state);
        }

        GPUVendor get_gpu_vendor(GraphicsDevice device)
        {
            DX12GraphicsDevice* dx12_device = (DX12GraphicsDevice*)device;
            return dx12_device->vendor;
        }

        const char* get_device_name(GraphicsDevice device)
        {
            DX12GraphicsDevice* dx12_device = (DX12GraphicsDevice*)device;
            return dx12_device->adapterName.c_str();
        }

        bool ray_tracing_support(GraphicsDevice device)
        {
            DX12GraphicsDevice* dx12_device = (DX12GraphicsDevice*)device;
            return dx12_device->supportRayTracing;
        }

        bool wave_mma_support(GraphicsDevice device)
        {
            DX12GraphicsDevice* dx12_device = (DX12GraphicsDevice*)device;
            return dx12_device->supportWaveMMA;
        }

        bool double_ops_support(GraphicsDevice device)
        {
            DX12GraphicsDevice* dx12_device = (DX12GraphicsDevice*)device;
            return dx12_device->supportDoubleShaderOps;
        }

        bool half_precision_support(GraphicsDevice device)
        {
            DX12GraphicsDevice* dx12_device = (DX12GraphicsDevice*)device;
            return dx12_device->support16bitShaderOps;
        }
    }
}

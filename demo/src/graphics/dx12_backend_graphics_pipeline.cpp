// Internal includes
#include "graphics/dx12_backend.h"
#include "graphics/dx12_containers.h"
#include "graphics/dx12_helpers.h"
#include "tools/string_utilities.h"
#include "tools/security.h"

namespace d3d12
{
    namespace graphics_pipeline
    {
        IDxcBlob* compile_kernel(IDxcCompiler* compiler, IDxcBlobEncoding* source_blob, const std::wstring& filename, const std::wstring& stageName, std::vector<LPCWSTR>& arguments, std::vector<DxcDefine>& defines, IDxcIncludeHandler* includeHandler, const wchar_t* profile)
        {
            // Compile the shader
            IDxcOperationResult* result;
            HRESULT hr = compiler->Compile(source_blob, filename.c_str(), stageName.c_str(), profile, arguments.data(), (uint32_t)arguments.size(), defines.data(), (uint32_t)defines.size(), includeHandler, &result);

            // Did we manage to compile it properly
            if (SUCCEEDED(hr)) result->GetStatus(&hr);
            bool compile_succeed = SUCCEEDED(hr);

            // If the compilation failed, print the error
            IDxcBlobEncoding* error_blob;
            if (SUCCEEDED(result->GetErrorBuffer(&error_blob)) && error_blob)
            {
                if (error_blob->GetBufferSize() != 0)
                    printf("[SHADER COMPILATION] Kernel %s %s\n", convert_to_regular(stageName).c_str(), (const char*)error_blob->GetBufferPointer());

                // Release the error blob
                error_blob->Release();
            }

            // If succeeded, grab the right pointer
            IDxcBlob* shader_blob = nullptr;
            if (compile_succeed) result->GetResult(&shader_blob);
            result->Release();

            // Return the result blob
            return shader_blob;
        }

        void fill_rasterizer_desc(const GraphicsPipelineDescriptor& gpd, D3D12_RASTERIZER_DESC& rasterDesc)
        {
            // Define the rasterization pipeline
            rasterDesc.FillMode = gpd.wireframe ? D3D12_FILL_MODE_WIREFRAME : D3D12_FILL_MODE_SOLID;
            rasterDesc.CullMode = (D3D12_CULL_MODE)gpd.cullMode;
            rasterDesc.FrontCounterClockwise = TRUE;
            rasterDesc.DepthBias = gpd.depthStencilState.enableDepth ? gpd.depthStencilState.depthBias : D3D12_DEFAULT_DEPTH_BIAS;
            rasterDesc.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
            rasterDesc.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
            rasterDesc.DepthClipEnable = TRUE;
            rasterDesc.MultisampleEnable = FALSE;
            rasterDesc.AntialiasedLineEnable = FALSE;
            rasterDesc.ForcedSampleCount = 0;
            rasterDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
        }

        void fill_blend_desc(const GraphicsPipelineDescriptor& gpd, D3D12_BLEND_DESC& blendDesc)
        {
            // Handle the color blending
            blendDesc.AlphaToCoverageEnable = FALSE;
            blendDesc.IndependentBlendEnable = FALSE;

            D3D12_RENDER_TARGET_BLEND_DESC rtBD;
            rtBD.BlendEnable = gpd.blendState.enableBlend;
            rtBD.LogicOpEnable = FALSE;
            rtBD.SrcBlend = (D3D12_BLEND)gpd.blendState.SrcBlend;
            rtBD.DestBlend = (D3D12_BLEND)gpd.blendState.DestBlend;
            rtBD.BlendOp = (D3D12_BLEND_OP)gpd.blendState.BlendOp;
            rtBD.SrcBlendAlpha = (D3D12_BLEND)gpd.blendState.SrcBlendAlpha;
            rtBD.DestBlendAlpha = (D3D12_BLEND)gpd.blendState.DestBlendAlpha;
            rtBD.BlendOpAlpha = (D3D12_BLEND_OP)gpd.blendState.BlendOpAlpha;
            rtBD.LogicOp = D3D12_LOGIC_OP_NOOP;
            rtBD.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

            // Set the bland state for each rendertarget
            for (UINT i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
                blendDesc.RenderTarget[i] = rtBD;
        }

        void fill_depth_stencil_desc(const DepthStencilState& ds, D3D12_DEPTH_STENCIL_DESC& dss)
        {
            // Handle the depth and stencil passes
            if (ds.enableDepth)
            {
                dss.DepthEnable = TRUE;
                dss.DepthWriteMask = ds.depthWrite ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
                dss.DepthFunc = (D3D12_COMPARISON_FUNC)ds.depthtest;
            }
            else
            {
                dss.DepthEnable = FALSE;
                dss.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
            }

            // Handle the stencil
            if (ds.enableStencil)
            {
                dss.StencilEnable = TRUE;
                dss.StencilReadMask = ds.stencilReadMask;
                dss.StencilWriteMask = ds.stencilWriteMask;

                dss.FrontFace.StencilPassOp = D3D12_STENCIL_OP_REPLACE;
                dss.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;

                dss.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
                dss.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;

                dss.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
                dss.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;

                dss.FrontFace.StencilFunc = (D3D12_COMPARISON_FUNC)ds.stencilTest;
                dss.BackFace.StencilFunc = D3D12_COMPARISON_FUNC::D3D12_COMPARISON_FUNC_ALWAYS;
            }
            else
            {
                dss.StencilEnable = FALSE;
            }
        }

        void fill_rt_format_array_desc(const GraphicsPipelineDescriptor& gpd, D3D12_RT_FORMAT_ARRAY& rtArray)
        {
            // Define the output of the pipelines
            rtArray.NumRenderTargets = gpd.numRenderTargets;
            for (uint32_t i = 0; i < gpd.numRenderTargets; ++i)
                rtArray.RTFormats[i] = format_to_dxgi_format(gpd.rtFormat[i]);
            for (uint32_t i = gpd.numRenderTargets; i < 8; ++i)
                rtArray.RTFormats[i] = DXGI_FORMAT_UNKNOWN;
        }

        void fill_sample_desc(const GraphicsPipelineDescriptor& gpd, DXGI_SAMPLE_DESC& sampleDesc)
        {
            sampleDesc.Count = 1;
            sampleDesc.Quality = 0;
        }

        GraphicsPipeline create_vertex_graphics_pipeline(GraphicsDevice graphicsDevice, const GraphicsPipelineDescriptor& gpd,
                                                            IDxcCompiler* compiler, IDxcLibrary* library, IDxcIncludeHandler* includeHandler,
                                                            std::vector<LPCWSTR>& arguments, std::vector<DxcDefine>& definesArray)
        {
            // Cast the graphics device
            DX12GraphicsDevice* deviceI = (DX12GraphicsDevice*)graphicsDevice;

            // File name
            const std::wstring& filename = convert_to_wide(gpd.filename.c_str(), (uint32_t)gpd.filename.size());

            // Vertex pipeline
            const std::wstring& vertexStage = convert_to_wide(gpd.vertexKernelName.c_str(), (uint32_t)gpd.vertexKernelName.size());
            const std::wstring& hullStage = convert_to_wide(gpd.hullKernelName.c_str(), (uint32_t)gpd.hullKernelName.size());
            const std::wstring& domainStage = convert_to_wide(gpd.domainKernelName.c_str(), (uint32_t)gpd.domainKernelName.size());
            const std::wstring& geometryStage = convert_to_wide(gpd.geometryKernelName.c_str(), (uint32_t)gpd.geometryKernelName.size());

            // Fragment shader
            const std::wstring& fragmentStage = convert_to_wide(gpd.fragmentKernelName.c_str(), (uint32_t)gpd.fragmentKernelName.size());

            // Load the file into a blob
            uint32_t code_page = CP_UTF8;
            IDxcBlobEncoding* source_blob;
            assert_msg(library->CreateBlobFromFile(filename.c_str(), &code_page, &source_blob) == S_OK, "Failed to load the shader code.");

            // Compile Vertex pipeline
            IDxcBlob* vertexBlob = compile_kernel(compiler, source_blob, filename, vertexStage, arguments, definesArray, includeHandler, L"vs_6_6");
            IDxcBlob* hullBlob = nullptr;
            if (hullStage != L"")
                hullBlob = compile_kernel(compiler, source_blob, filename, hullStage, arguments, definesArray, includeHandler, L"hs_6_6");
            IDxcBlob* domainBlob = nullptr;
            if (domainStage != L"")
                domainBlob = compile_kernel(compiler, source_blob, filename, domainStage, arguments, definesArray, includeHandler, L"ds_6_6");
            IDxcBlob* geometryBlob = nullptr;
            if (geometryStage != L"")
                geometryBlob = compile_kernel(compiler, source_blob, filename, geometryStage, arguments, definesArray, includeHandler, L"gs_6_6");

            // Compile the fragment shader
            IDxcBlob* fragmentBlob = compile_kernel(compiler, source_blob, filename, fragmentStage, arguments, definesArray, includeHandler, L"ps_6_6");

            // Release all the intermediate resources
            library->Release();
            source_blob->Release();
            compiler->Release();

            // If one of the two stages did not compile properly, leave
            if (vertexBlob == nullptr || fragmentBlob == nullptr)
                return 0;

            // PSO descriptor that we need to fill
            D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};

            // Input elements for the vertex shader
            D3D12_INPUT_ELEMENT_DESC inputElementDescs[] = {
                {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
                {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
                {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
            };
            psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };

            // Node mask
            psoDesc.NodeMask = 0x1;

            // Create the root signature for the pipeline
            DX12GraphicsPipeline* dx12_graphicsPipeline = new DX12GraphicsPipeline();
            // Vertex pipeline
            dx12_graphicsPipeline->vertBlob = vertexBlob;
            dx12_graphicsPipeline->hullblob = hullBlob;
            dx12_graphicsPipeline->domainBlob = domainBlob;
            dx12_graphicsPipeline->geometryBlob = geometryBlob;
            // Fragment pipeline
            dx12_graphicsPipeline->fragBlob = fragmentBlob;

            // Root signature
            dx12_graphicsPipeline->rootSignature = create_root_signature(deviceI, gpd.srvCount, gpd.uavCount, gpd.cbvCount, gpd.samplerCount);
            psoDesc.pRootSignature = dx12_graphicsPipeline->rootSignature->rootSignature;

            // Handle the vertex shader
            D3D12_SHADER_BYTECODE vsBytecode;
            vsBytecode.pShaderBytecode = vertexBlob->GetBufferPointer();
            vsBytecode.BytecodeLength = vertexBlob->GetBufferSize();
            psoDesc.VS = vsBytecode;

            if (hullBlob != nullptr)
            {
                D3D12_SHADER_BYTECODE hlBytecode;
                hlBytecode.pShaderBytecode = hullBlob->GetBufferPointer();
                hlBytecode.BytecodeLength = hullBlob->GetBufferSize();
                psoDesc.HS = hlBytecode;
            }

            if (domainBlob != nullptr)
            {
                D3D12_SHADER_BYTECODE dmBytecode;
                dmBytecode.pShaderBytecode = domainBlob->GetBufferPointer();
                dmBytecode.BytecodeLength = domainBlob->GetBufferSize();
                psoDesc.DS = dmBytecode;
            }

            if (geometryBlob != nullptr)
            {
                // Handle the fragment shader
                D3D12_SHADER_BYTECODE gBytecode;
                gBytecode.pShaderBytecode = geometryBlob->GetBufferPointer();
                gBytecode.BytecodeLength = geometryBlob->GetBufferSize();
                psoDesc.GS = gBytecode;
            }

            // Handle the fragment shader
            D3D12_SHADER_BYTECODE psBytecode;
            psBytecode.pShaderBytecode = fragmentBlob->GetBufferPointer();
            psBytecode.BytecodeLength = fragmentBlob->GetBufferSize();
            psoDesc.PS = psBytecode;

            // Define the rasterization pipeline
            D3D12_RASTERIZER_DESC rasterDesc;
            fill_rasterizer_desc(gpd, rasterDesc);
            psoDesc.RasterizerState = rasterDesc;
            psoDesc.PrimitiveTopologyType = (hullBlob != nullptr && domainBlob != nullptr) ? D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH : D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

            // Handle the color blending
            D3D12_BLEND_DESC blendDesc;
            fill_blend_desc(gpd, blendDesc);
            psoDesc.BlendState = blendDesc;

            // Fill the depth stencil state
            D3D12_DEPTH_STENCIL_DESC dss;
            fill_depth_stencil_desc(gpd.depthStencilState, dss);
            psoDesc.DepthStencilState = dss;

            // Set the format
            if (gpd.depthStencilState.enableDepth || gpd.depthStencilState.enableStencil)
                psoDesc.DSVFormat = sanitize_dxgi_format_clear(format_to_dxgi_format(gpd.depthStencilState.depthStencilFormat));

            psoDesc.SampleMask = UINT_MAX;

            // Define the output of the pipelines
            psoDesc.NumRenderTargets = gpd.numRenderTargets;
            for (uint32_t i = 0; i < gpd.numRenderTargets; ++i)
                psoDesc.RTVFormats[i] = format_to_dxgi_format(gpd.rtFormat[i]);
            for (uint32_t i = gpd.numRenderTargets; i < 8; ++i)
                psoDesc.RTVFormats[i] = DXGI_FORMAT_UNKNOWN;

            // Sample Desc
            DXGI_SAMPLE_DESC sd;
            fill_sample_desc(gpd, sd);
            psoDesc.SampleDesc = sd;

            // Cast the graphics device
            dx12_graphicsPipeline->device = deviceI;

            // Create the pipeline state
            HRESULT hr = deviceI->device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&dx12_graphicsPipeline->pipelineStateObject));
            assert_msg(hr == S_OK, "Failed to create graphics pipeline state.");

            // Create the command signature
            D3D12_INDIRECT_ARGUMENT_DESC argumentDescs[1];
            argumentDescs[0].Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW;
            D3D12_COMMAND_SIGNATURE_DESC commandSignatureDesc = {};
            commandSignatureDesc.pArgumentDescs = argumentDescs;
            commandSignatureDesc.NumArgumentDescs = 1;
            commandSignatureDesc.ByteStride = sizeof(D3D12_DRAW_ARGUMENTS);
            assert(deviceI->device->CreateCommandSignature(&commandSignatureDesc, nullptr, IID_PPV_ARGS(&dx12_graphicsPipeline->commandSignature)) == S_OK);

            // Create the descriptor heap for this compute shader
            dx12_graphicsPipeline->CSUHeaps.push_back(create_descriptor_heap_suc(deviceI, gpd.srvCount, gpd.uavCount, gpd.cbvCount));
            dx12_graphicsPipeline->samplerHeaps.push_back(create_descriptor_heap_sampler(deviceI, max(1, gpd.samplerCount)));
            dx12_graphicsPipeline->cmdBatchIndex = UINT32_MAX;
            dx12_graphicsPipeline->nextUsableHeap = 0;
            dx12_graphicsPipeline->srvCount = gpd.srvCount;
            dx12_graphicsPipeline->uavCount = gpd.uavCount;
            dx12_graphicsPipeline->cbvCount = gpd.cbvCount;
            dx12_graphicsPipeline->samplerCount = gpd.samplerCount;
            dx12_graphicsPipeline->stencilRef = gpd.depthStencilState.stencilRef;

            // Convert to the opaque structure
            return (GraphicsPipeline)dx12_graphicsPipeline;
        }

        GraphicsPipeline create_graphics_pipeline(GraphicsDevice graphicsDevice, const GraphicsPipelineDescriptor& gpd)
        {
            // Cast the graphics device
            DX12GraphicsDevice* deviceI = (DX12GraphicsDevice*)graphicsDevice;

            // Handle the include directories
            std::vector<std::wstring> includeDirs;
            for (int includeDirIdx = 0; includeDirIdx < gpd.includeDirectories.size(); ++includeDirIdx)
            {
                std::wstring includeArg = L"-I ";
                includeArg += convert_to_wide(gpd.includeDirectories[includeDirIdx]);
                includeDirs.push_back(includeArg.c_str());
            }

            std::vector<LPCWSTR> arguments;
#ifndef DEBUG_SHADERS
            arguments.push_back(L"-O3");
#else
            arguments.push_back(L"-Od");
            arguments.push_back(L"-Fd");
            arguments.push_back(L"-Qembed_debug");
            arguments.push_back(L"-Zi");
#endif
            arguments.push_back(L"-enable-16bit-types");

            // Handle the defines
            std::vector<DxcDefine> definesArray(gpd.defines.size());
            std::vector<std::wstring> definesWSTR(gpd.defines.size());
            for (int defIdx = 0; defIdx < gpd.defines.size(); ++defIdx)
            {
                // Convert and keep it for memory management issues
                definesWSTR[defIdx] = convert_to_wide(gpd.defines[defIdx]);

                // Add the define
                DxcDefine def;
                def.Name = definesWSTR[defIdx].c_str();
                def.Value = L"1";
                definesArray[defIdx] = def;
            }

            // Double unsupported on intel
            if (deviceI->vendor == GPUVendor::Intel)
            {
                DxcDefine def;
                def.Name = L"FP64_UNSUPPORTED";
                def.Value = L"1";
                definesArray.push_back(def);
                def.Name = L"UNSUPPORTED_BARYCENTRICS";
                def.Value = L"1";
                definesArray.push_back(def);
            }

            // Handle the include dirs
            for (int includeDirIdx = 0; includeDirIdx < gpd.includeDirectories.size(); ++includeDirIdx)
                arguments.push_back(includeDirs[includeDirIdx].c_str());

            // Create the compiler
            IDxcCompiler* compiler;
            DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&compiler));

            // Create the library
            IDxcLibrary* library;
            DxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(&library));

            // Create an include handler
            IDxcIncludeHandler* includeHandler;
            library->CreateIncludeHandler(&includeHandler);

            return create_vertex_graphics_pipeline(graphicsDevice, gpd, compiler, library, includeHandler, arguments, definesArray);
        }

        void destroy_graphics_pipeline(GraphicsPipeline graphicsPipeline)
        {
            // Grab the internal structure
            DX12GraphicsPipeline* dx12_gp = (DX12GraphicsPipeline*)graphicsPipeline;

            // Destroy all the descriptor heaps
            uint32_t numDescriptorHeaps = (uint32_t)dx12_gp->CSUHeaps.size();
            for (uint32_t heapIdx = 0; heapIdx < numDescriptorHeaps; ++heapIdx)
            {
                destroy_descriptor_heap(dx12_gp->CSUHeaps[heapIdx]);
                destroy_descriptor_heap(dx12_gp->samplerHeaps[heapIdx]);
            }

            // Destroy the dx12 objects
            dx12_gp->commandSignature->Release();
            dx12_gp->pipelineStateObject->Release();
            destroy_root_signature(dx12_gp->rootSignature);

            // Vertex pipeline
            if (dx12_gp->vertBlob != nullptr)
                dx12_gp->vertBlob->Release();
            if (dx12_gp->hullblob != nullptr)
                dx12_gp->hullblob->Release();
            if (dx12_gp->domainBlob != nullptr)
                dx12_gp->domainBlob->Release();
            if (dx12_gp->geometryBlob != nullptr)
                dx12_gp->geometryBlob->Release();

            // Mesh pipeline
            if (dx12_gp->ampliBlob != nullptr)
                dx12_gp->ampliBlob->Release();
            if (dx12_gp->meshBlob != nullptr)
                dx12_gp->meshBlob->Release();

            // Fragment shader
            if (dx12_gp->fragBlob != nullptr)
                dx12_gp->fragBlob->Release();

            // Destroy the internal structure
            delete dx12_gp;
        }

        void set_stencil_ref(GraphicsPipeline graphicsPipeline, uint8_t stencilRef)
        {
            // Grab the internal structure
            DX12GraphicsPipeline* dx12_gp = (DX12GraphicsPipeline*)graphicsPipeline;
            dx12_gp->stencilRef = stencilRef;
        }
    }
}

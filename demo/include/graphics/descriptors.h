#pragma once

// Project includes
#include "graphics/types.h"
#include "math/types.h"

// System includes
#include <string>
#include <vector>

// Descriptor for a given sampler
struct SamplerDescriptor
{
	FilterMode filterMode = FilterMode::Linear;
	SamplerMode modeX = SamplerMode::Wrap;
	SamplerMode modeY = SamplerMode::Wrap;
	SamplerMode modeZ = SamplerMode::Wrap;
	uint8_t anisotropy = 1;
};

// Descriptor for a given texture
struct TextureDescriptor
{
	// Dimension of the texture
	TextureType type = TextureType::Count;

	// Dimensions of the texture
	uint32_t width = 0;
	uint32_t height = 0;
	uint32_t depth = 0;

	// Number of mips
	uint32_t mipCount = 0;

	// Is the texture an UAV?
	bool isUAV = false;

	// Format
	TextureFormat format = TextureFormat::Count;

	// Clear color
	float4 clearColor = { 0.0, 0.0, 0.0, 0.0 };

	// Optional debug name
	std::string debugName = "";
};

// Structure that describes a compute shader input
struct ComputeShaderDescriptor
{
	// File where the compute shader is stored
	std::string filename = "";

	// Name of the kernel in the target file
	std::string kernelname = "";

	// List of include directories for the compute shader
	std::vector<std::string> includeDirectories;

	// List of defines for the compute shader
	std::vector<std::string> defines;

	// Inputs of the compute shader
	uint32_t uavCount = 0;
	uint32_t srvCount = 0;
	uint32_t cbvCount = 0;
	uint32_t samplerCount = 0;
};

// Descriptor for the whole blend state
struct BlendState
{
	bool enableBlend = false;
	BlendFactor SrcBlend = BlendFactor::One;
	BlendFactor DestBlend = BlendFactor::Zero;
	BlendOperator BlendOp = BlendOperator::Add;
	BlendFactor SrcBlendAlpha = BlendFactor::One;
	BlendFactor DestBlendAlpha = BlendFactor::Zero;
	BlendOperator BlendOpAlpha = BlendOperator::Add;
};

struct DepthStencilState
{
	// Common parameters
	bool enableDepth = false;
	bool enableStencil = false;
	TextureFormat depthStencilFormat = TextureFormat::Depth32Stencil8;

	// Depth parameters
	DepthTest depthtest = DepthTest::Always;
	bool depthWrite = true;
	int32_t depthBias = 0;

	// Stencil test
	StencilTest stencilTest = StencilTest::Always;
	uint8_t stencilWriteMask = 0xff;
	uint8_t stencilReadMask = 0xff;
	uint8_t stencilRef = 0;
	StencilOp stencilOperation = StencilOp::Keep;
};

struct GraphicsPipelineDescriptor
{
	// File where the stages are stored
	std::string filename;

	// Vertex pipeline
	std::string vertexKernelName = "vert";
	std::string hullKernelName = "";
	std::string domainKernelName = "";
	std::string geometryKernelName = "";

	// Fragment shader
	std::string fragmentKernelName = "frag";

	// List of defines for the graphics pipeline
	std::vector<std::string> defines;

	// Include Dirs
	std::vector<std::string> includeDirectories;

	// Render target formats
	uint32_t numRenderTargets = 1;
	TextureFormat rtFormat[8] = { TextureFormat::R16G16B16A16_Float };

	// Resource count
	uint32_t uavCount = 0;
	uint32_t srvCount = 0;
	uint32_t cbvCount = 0;
	uint32_t samplerCount = 0;

	// Depth stencil state
	DepthStencilState depthStencilState = { false, false };

	// Blend mode
	BlendState blendState = { false };

	// Rendering properties of the pipeline
	bool wireframe = false;
	CullMode cullMode = CullMode::Back;
};

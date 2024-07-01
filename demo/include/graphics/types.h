#pragma once

// Define to be able to debug shaders
// #define DEBUG_SHADERS 1

// External includes
#include <stdint.h>

// General graphics objects
typedef uint64_t RenderWindow;
typedef uint64_t GraphicsDevice;
typedef uint64_t CommandQueue;
typedef uint64_t SwapChain;
typedef uint64_t CommandBuffer;

// Shaders objects
typedef uint64_t ComputeShader;
typedef uint64_t GraphicsPipeline;
typedef uint64_t RayTracingShader;

// Syncrhonization
typedef uint64_t Fence;

// Resources objects
typedef uint64_t Texture;
typedef uint64_t RenderTexture;
typedef uint64_t GraphicsBuffer;
typedef uint64_t ConstantBuffer;
typedef uint64_t TopLevelAS;
typedef uint64_t BottomLevelAS;

// Profiling
typedef uint64_t ProfilingScope;

// Device pick strategy
enum class DevicePickStrategy
{
    VRAMSize = 0,
    VendorID,
    AdapterID,
	Count
};

// Vendors for the GPUs
enum class GPUVendor
{
    Intel = 0,
    AMD,
    Nvidia,
    Other,
	Count
};

// Types of constant buffers
enum class ConstantBufferType
{
    Static = 0x01,
    Runtime = 0x02,
    Mixed = 0x03
};

// Types of graphics buffers
enum class GraphicsBufferType
{
    Default = 0,
    Upload,
    Readback,
    RTAS,
    Count
};

// Types of the command buffer
enum class CommandBufferType
{
    Default = 0,
    Compute = 2,
    Copy = 3
};

// Command queue priority
enum class CommandQueuePriority
{
    Normal = 0,
    High = 1,
    Realtime = 2,
	Count
};

// Types of textures
enum class TextureType
{
	Tex1D,
	Tex1DArray,
	Tex2D,
	Tex2DArray,
	Tex3D,
	TexCube,
	TexCubeArray,
	Count
};

// Texture formats supported
enum class TextureFormat
{
	// 8 Formats
	R8_SNorm,
	R8_UNorm,
	R8_SInt,
	R8_UInt,
	R8G8_SNorm,
	R8G8_UNorm,
	R8G8_SInt,
	R8G8_UInt,
	R8G8B8A8_SNorm,
	R8G8B8A8_UNorm,
	R8G8B8A8_UNorm_SRGB,
	R8G8B8A8_UInt,
	R8G8B8A8_SInt,

	// 16 Formats
	R16_Float,
	R16_SInt,
	R16_UInt,
	R16G16_Float,
	R16G16_SInt,
	R16G16_UInt,
	R16G16B16A16_Float,
	R16G16B16A16_UInt,
	R16G16B16A16_SInt,

	// 32 Formats
	R32_Float,
	R32_SInt,
	R32_UInt,
	R32G32_Float,
	R32G32_SInt,
	R32G32_UInt,
	R32G32B32A32_Float,
	R32G32B32A32_UInt,
	R32G32B32A32_SInt,

	// Depth buffer formats
	Depth32,
	Depth32Stencil8,
	Depth24Stencil8,

	// Other Formats
	R10G10B10A2_UNorm,
	R10G10B10A2_UInt,
	R11G11B10_Float,

	Count
};

enum class FilterMode
{
	Point = 0,
	Linear,
	Anisotropic,
	Count
};

enum class SamplerMode
{
	Wrap = 1,
	Mirror,
	Clamp,
	Border,
	MirrorOnce,
	Count
};


enum class DepthTest
{
	Never = 1,
	Less,
	Equal,
	LEqual,
	Greater,
	NotEqual,
	GEqual,
	Always,
};

enum class StencilTest
{
	Never = 1,
	Less,
	Equal,
	LEqual,
	Greater,
	NotEqual,
	GEqual,
	Always,
};

enum class StencilOp
{
	Keep = 1,
	Zero = 2,
	Replace = 3,
	IncrementClamp = 4,
	DecrementClamp = 5,
	Invert = 6,
	IncrementWrap = 7,
	DecrementWrap = 8
};

enum class BlendOperator
{
	Add = 1,
	Subtract,
	RevSubstract,
	Min,
	Max
};

enum class BlendFactor
{
	Zero = 1,
	One = 2,
	SrcColor = 3,
	InvSrcColor = 4,
	SrcAlpha = 5,
	InvSrcAlpha = 6,
	DestAlpha = 7,
	InvDestAlpha = 8,
	DestColor = 9,
	InvDestColor = 10
};

enum class CullMode
{
	None = 1,
	Front = 2,
	Back = 3,
};

enum class DrawPrimitive
{
	Line,
	Triangle
};

// For C++, just output as is
#define CBV_SLOT(X) X
#define SRV_SLOT(X) X
#define UAV_SLOT(X) X
#define SPL_SLOT(X) X
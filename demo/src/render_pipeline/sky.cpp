// Bacasable includes
#include "graphics/dx12_backend.h"
#include "math/operators.h"
#include "tools/texture_utils.h"
#include "tools/shader_utils.h"

// Project includes
#include "render_pipeline/sky.h"
#include "render_pipeline/stencil_usage.h"

// Shader file and kernels
const char* sky_pre_compute_file = "SkyPreCompute.compute";
const char* transmittance_lut_kernel = "TransmittanceLUT";
const char* multi_scattering_lut_kernel = "MultiScatteringLUT";
const char* sky_view_lut_kernel = "SkyViewLUT";

const char* render_sky_graphics_file = "RenderSky.graphics";
const char* render_sky_fast_graphics_file = "RenderSkyFast.graphics";

// Resolution of the luts
const uint32_t transmittance_lut_width = 256;
const uint32_t transmittance_lut_height = 64;
const uint32_t multi_scat_lut_size = 32;
const uint32_t sky_view_lut_width = 192;
const uint32_t sky_view_lut_height = 108;

// CBVs
#define GLOBAL_CB_BINDING_SLOT CBV_SLOT(0)
#define SKY_ATMOSPHERE_BUFFER_SLOT CBV_SLOT(1)
#define RENDER_SKY_CBV_COUNT 2

// SRVs
#define TRANSMITTANCE_LUT_TEXTURE_SLOT SRV_SLOT(0)
#define MULTI_SCATTERING_LUT_TEXTURE_SLOT SRV_SLOT(1)
#define SKY_VIEW_LUT_TEXTURE_SLOT SRV_SLOT(2)
#define DEPTH_BUFFER_TEXTURE_SLOT SRV_SLOT(3)
#define RENDER_SKY_SRV_COUNT 4

// Samplers
#define LINEAR_CLAMP_SAMPLER_SLOT SPL_SLOT(0)
#define RENDER_SKY_SPL_COUNT 4

struct SkyAtmosphereCB
{
	float3  _AbsorptionExtinction;
	float _PlanetRadiusSky;

	float3  _RayleighScattering;
	float   _MisePhaseFunctionG;

	float3  _MieScattering;
	float   _BottomRadius;

	float3  _MieExtinction;
	float   _TopRadius;

	float3  _MieAbsorption;
	float   _MiePhaseG;

	float3 _GroundAlbedo;
	float _PaddingSA0;

	float3 _PlanetCenterSky;
	float _PaddingSA1;

	float _RayleighDensity0LayerWidth;
	float _RayleighDensity0ExpTerm;
	float _RayleighDensity0ExpScale;
	float _RayleighDensity0LinearTerm;
	float _RayleighDensity0ConstantTerm;

	float _RayleighDensity1LayerWidth;
	float _RayleighDensity1ExpTerm;
	float _RayleighDensity1ExpScale;
	float _RayleighDensity1LinearTerm;
	float _RayleighDensity1ConstantTerm;

	float _MieDensity0LayerWidth;
	float _MieDensity0ExpTerm;
	float _MieDensity0ExpScale;
	float _MieDensity0LinearTerm;
	float _MieDensity0ConstantTerm;

	float _MieDensity1LayerWidth;
	float _MieDensity1ExpTerm;
	float _MieDensity1ExpScale;
	float _MieDensity1LinearTerm;
	float _MieDensity1ConstantTerm;

	float _AbsorptionDensity0LayerWidth;
	float _AbsorptionDensity0ExpTerm;
	float _AbsorptionDensity0ExpScale;
	float _AbsorptionDensity0LinearTerm;
	float _AbsorptionDensity0ConstantTerm;

	float _AbsorptionDensity1LayerWidth;
	float _AbsorptionDensity1ExpTerm;
	float _AbsorptionDensity1ExpScale;
	float _AbsorptionDensity1LinearTerm;
	float _AbsorptionDensity1ConstantTerm;
};

Sky::Sky()
{
}

Sky::~Sky()
{
}

void Sky::initialize(GraphicsDevice device)
{
	// Keep track of the device
	m_Device = device;

	// Buffers that hold the precomputations
	TextureDescriptor texDesc;
	texDesc.type = TextureType::Tex2D;
	texDesc.depth = 1;
	texDesc.mipCount = 1;
	texDesc.isUAV = true;
	texDesc.format = TextureFormat::R16G16B16A16_Float;
	texDesc.clearColor = float4({0.0, 0.0, 0.0, 0.0});

	// Create the transmittance lut
	texDesc.width = transmittance_lut_width;
	texDesc.height = transmittance_lut_height;
	m_TransmittanceLutTex = d3d12::graphics_resources::create_texture(device, texDesc);

	// Create the multi scattering lut
	texDesc.width = multi_scat_lut_size;
	texDesc.height = multi_scat_lut_size;
	m_MultiScatteringLutTex = d3d12::graphics_resources::create_texture(device, texDesc);

	// Create the sky view lut
	texDesc.width = sky_view_lut_width;
	texDesc.height = sky_view_lut_height;
	m_SkyViewLutTex = d3d12::graphics_resources::create_texture(device, texDesc);

	// Constant buffer
	m_SkyAtmosphereCB = d3d12::graphics_resources::create_constant_buffer(device, sizeof(SkyAtmosphereCB), ConstantBufferType::Mixed);
}

void Sky::release()
{
	d3d12::graphics_resources::destroy_constant_buffer(m_SkyAtmosphereCB);

	d3d12::graphics_pipeline::destroy_graphics_pipeline(m_SkyRendererFastGP);
	d3d12::graphics_pipeline::destroy_graphics_pipeline(m_SkyRendererGP);

	d3d12::compute_shader::destroy_compute_shader(m_SkyViewLutCS);
	d3d12::compute_shader::destroy_compute_shader(m_MultiScatteringLutCS);
	d3d12::compute_shader::destroy_compute_shader(m_TransmittanceLutCS);

	d3d12::graphics_resources::destroy_texture(m_SkyViewLutTex);
	d3d12::graphics_resources::destroy_texture(m_MultiScatteringLutTex);
	d3d12::graphics_resources::destroy_texture(m_TransmittanceLutTex);
}

void Sky::reload_shaders(const std::string& shaderLibrary)
{
	// Create the compute shaders
	ComputeShaderDescriptor csd;
	csd.includeDirectories.push_back(shaderLibrary);
	csd.filename = shaderLibrary + "\\Sky\\" + sky_pre_compute_file;
	csd.srvCount = 2;
	csd.uavCount = 3;
	csd.cbvCount = 2;
	csd.samplerCount = 1;

	// Transmittance Lut kernel
	csd.kernelname = transmittance_lut_kernel;
	compile_and_replace_compute_shader(m_Device, csd, m_TransmittanceLutCS);

	// Multi Scat Lut kernel
	csd.kernelname = multi_scattering_lut_kernel;
	compile_and_replace_compute_shader(m_Device, csd, m_MultiScatteringLutCS);

	// Multi Scat Lut kernel
	csd.kernelname = sky_view_lut_kernel;
	compile_and_replace_compute_shader(m_Device, csd, m_SkyViewLutCS);

	// Sky GP
	{
		GraphicsPipelineDescriptor gpd;
		gpd.filename = shaderLibrary + "\\Sky\\" + render_sky_fast_graphics_file;
		gpd.includeDirectories.push_back(shaderLibrary);
		gpd.cbvCount = RENDER_SKY_CBV_COUNT;
		gpd.srvCount = RENDER_SKY_SRV_COUNT;
		gpd.samplerCount = RENDER_SKY_SPL_COUNT;
		gpd.blendState.enableBlend = true;
		gpd.blendState.SrcBlend = BlendFactor::One;
		gpd.blendState.DestBlend = BlendFactor::SrcAlpha;
		gpd.blendState.BlendOp = BlendOperator::Add;

		// Stencil
		gpd.depthStencilState.enableStencil = true;
		gpd.depthStencilState.depthStencilFormat = TextureFormat::Depth32Stencil8;
		gpd.depthStencilState.stencilWriteMask = 0;
		gpd.depthStencilState.stencilReadMask = (uint32_t)StencilUsage::SkyOccluder | (uint32_t)StencilUsage::SkyHost;
		gpd.depthStencilState.stencilTest = StencilTest::Equal;
		gpd.depthStencilState.stencilRef = 0x0;
		compile_and_replace_graphics_pipeline(m_Device, gpd, m_SkyRendererFastGP);
	}

	// Sky GP
	{
		GraphicsPipelineDescriptor gpd;
		gpd.filename = shaderLibrary + "\\Sky\\" + render_sky_graphics_file;
		gpd.includeDirectories.push_back(shaderLibrary);
		gpd.cbvCount = RENDER_SKY_CBV_COUNT;
		gpd.srvCount = RENDER_SKY_SRV_COUNT;
		gpd.samplerCount = RENDER_SKY_SPL_COUNT;
		gpd.blendState.enableBlend = true;
		gpd.blendState.SrcBlend = BlendFactor::One;
		gpd.blendState.DestBlend = BlendFactor::SrcAlpha;
		gpd.blendState.BlendOp = BlendOperator::Add;

		// Stencil
		gpd.depthStencilState.enableStencil = true;
		gpd.depthStencilState.depthStencilFormat = TextureFormat::Depth32Stencil8;
		gpd.depthStencilState.stencilWriteMask = 0;
		gpd.depthStencilState.stencilReadMask = (uint32_t)StencilUsage::SkyOccluder | (uint32_t)StencilUsage::SkyHost;
		gpd.depthStencilState.stencilTest = StencilTest::Equal;
		gpd.depthStencilState.stencilRef = (uint32_t)StencilUsage::SkyHost;
		compile_and_replace_graphics_pipeline(m_Device, gpd, m_SkyRendererGP);
	}
}

// Pre-rendering steps
void Sky::pre_render(CommandBuffer cmdB, ConstantBuffer globalCB, float planetRadius, float3 planetCenter)
{
	d3d12::command_buffer::start_section(cmdB, "Prepare Sky");
	{
		// Update the constant buffers
		update_constant_buffer(cmdB,planetRadius, planetCenter);

		// Evaluate the transmittance LUT
		{
			// CBVs
			d3d12::command_buffer::set_compute_shader_buffer_cbv(cmdB, m_TransmittanceLutCS, 0, globalCB);
			d3d12::command_buffer::set_compute_shader_buffer_cbv(cmdB, m_TransmittanceLutCS, 1, m_SkyAtmosphereCB);

			// UAVs
			d3d12::command_buffer::set_compute_shader_texture_uav(cmdB, m_TransmittanceLutCS, 0, m_TransmittanceLutTex);

			// Dispatch
			d3d12::command_buffer::dispatch(cmdB, m_TransmittanceLutCS, transmittance_lut_width / 8, transmittance_lut_height / 8, 1);

			// Barrier
			d3d12::command_buffer::uav_barrier_texture(cmdB, m_TransmittanceLutTex);
		}

		// Evaluate the multi scattering LUT
		{
			// CBVs
			d3d12::command_buffer::set_compute_shader_buffer_cbv(cmdB, m_MultiScatteringLutCS, 0, globalCB);
			d3d12::command_buffer::set_compute_shader_buffer_cbv(cmdB, m_MultiScatteringLutCS, 1, m_SkyAtmosphereCB);

			// SRVs
			d3d12::command_buffer::set_compute_shader_texture_srv(cmdB, m_MultiScatteringLutCS, 0, m_TransmittanceLutTex);

			// UAVs
			d3d12::command_buffer::set_compute_shader_texture_uav(cmdB, m_MultiScatteringLutCS, 1, m_MultiScatteringLutTex);

			// Samplers
			SamplerDescriptor sampler({ FilterMode::Linear, SamplerMode::Clamp, SamplerMode::Clamp, SamplerMode::Clamp });
			d3d12::command_buffer::set_compute_shader_sampler(cmdB, m_MultiScatteringLutCS, 0, sampler);

			// Dispatch
			d3d12::command_buffer::dispatch(cmdB, m_MultiScatteringLutCS, multi_scat_lut_size, multi_scat_lut_size, 1);

			// Barrier
			d3d12::command_buffer::uav_barrier_texture(cmdB, m_MultiScatteringLutTex);
		}
	}
	d3d12::command_buffer::end_section(cmdB);
}

void Sky::update_constant_buffer(CommandBuffer cmdB, float planetRadius, float3 planetCenter)
{
	SkyAtmosphereCB skyCB;
	skyCB._AbsorptionExtinction = float3({ 0.000650f, 0.001881f, 0.000085f });
	skyCB._RayleighScattering = float3({ 0.005802f, 0.013558f, 0.033100f });
	skyCB._MisePhaseFunctionG = 0.8f;
	skyCB._MieScattering = float3({ 0.003996f, 0.003996f, 0.003996f });
	skyCB._PlanetRadiusSky = planetRadius;
	skyCB._PlanetCenterSky = { planetCenter.x / 1000.0f, planetCenter.y / 1000.0f, planetCenter.z / 1000.0f };
	skyCB._BottomRadius = planetRadius / 1000.0f - 1.0f;
	skyCB._MieExtinction = float3({ 0.004440f, 0.004440f, 0.004440f });
	skyCB._TopRadius = skyCB._BottomRadius * 1.013f;
	skyCB._MieAbsorption = max_zero(skyCB._MieExtinction - skyCB._MieScattering);
	skyCB._MiePhaseG = 0.8;
	skyCB._GroundAlbedo = float3({0.0, 0.01, 0.02});

	skyCB._RayleighDensity0LayerWidth = 0.0f;
	skyCB._RayleighDensity0ExpTerm = 0.0f;
	skyCB._RayleighDensity0ExpScale = 0.0f;
	skyCB._RayleighDensity0LinearTerm = 0.0f;
	skyCB._RayleighDensity0ConstantTerm = 0.0f;

	skyCB._RayleighDensity1LayerWidth = 0.0f;
	skyCB._RayleighDensity1ExpTerm = 1.0f;
	skyCB._RayleighDensity1ExpScale = -1.0f / 8.0f;
	skyCB._RayleighDensity1LinearTerm = 0.0f;
	skyCB._RayleighDensity1ConstantTerm = 0.0f;

	skyCB._MieDensity0LayerWidth = 0.0f;
	skyCB._MieDensity0ExpTerm = 0.0f;
	skyCB._MieDensity0ExpScale = 0.0f;
	skyCB._MieDensity0LinearTerm = 0.0f;
	skyCB._MieDensity0ConstantTerm = 0.0f;

	skyCB._MieDensity1LayerWidth = 0.0f;
	skyCB._MieDensity1ExpTerm = 1.0f;
	skyCB._MieDensity1ExpScale = -1.0f / 1.2f;
	skyCB._MieDensity1LinearTerm = 0.0f;
	skyCB._MieDensity1ConstantTerm = 0.0f;

	skyCB._AbsorptionDensity0LayerWidth = 25.0f;
	skyCB._AbsorptionDensity0ExpTerm = 0.0f;
	skyCB._AbsorptionDensity0ExpScale = 0.0f;
	skyCB._AbsorptionDensity0LinearTerm = 1.0f / 15.0f;
	skyCB._AbsorptionDensity0ConstantTerm = -2.0f / 3.0f;

	skyCB._AbsorptionDensity1LayerWidth = 0.0f;
	skyCB._AbsorptionDensity1ExpTerm = 0.0f;
	skyCB._AbsorptionDensity1ExpScale = 0.0f;
	skyCB._AbsorptionDensity1LinearTerm = -1.0f / 15.0f;
	skyCB._AbsorptionDensity1ConstantTerm = 8.0f / 3.0f;

	d3d12::graphics_resources::set_constant_buffer(m_SkyAtmosphereCB, (const char*)&skyCB, sizeof(SkyAtmosphereCB));
	d3d12::command_buffer::upload_constant_buffer(cmdB, m_SkyAtmosphereCB);
}

void Sky::update_sky(CommandBuffer cmdB, ConstantBuffer globalCB, float planetRadius, float3 planetCenter)
{
	d3d12::command_buffer::start_section(cmdB, "Update Sky");
	{
		// Update the constant buffers
		update_constant_buffer(cmdB, planetRadius, planetCenter);

		// CBVs
		d3d12::command_buffer::set_compute_shader_buffer_cbv(cmdB, m_SkyViewLutCS, 0, globalCB);
		d3d12::command_buffer::set_compute_shader_buffer_cbv(cmdB, m_SkyViewLutCS, 1, m_SkyAtmosphereCB);

		// SRVs
		d3d12::command_buffer::set_compute_shader_texture_srv(cmdB, m_SkyViewLutCS, 0, m_TransmittanceLutTex);
		d3d12::command_buffer::set_compute_shader_texture_srv(cmdB, m_SkyViewLutCS, 1, m_MultiScatteringLutTex);

		// UAVs
		d3d12::command_buffer::set_compute_shader_texture_uav(cmdB, m_SkyViewLutCS, 2, m_SkyViewLutTex);

		// Sampler
		SamplerDescriptor sampler({ FilterMode::Linear, SamplerMode::Clamp, SamplerMode::Clamp, SamplerMode::Clamp });
		d3d12::command_buffer::set_compute_shader_sampler(cmdB, m_SkyViewLutCS, 0, sampler);

		// Dispatch
		d3d12::command_buffer::dispatch(cmdB, m_SkyViewLutCS, (sky_view_lut_width + 7) / 8, (sky_view_lut_height + 7) / 8, 1);

		// Barrier
		d3d12::command_buffer::uav_barrier_texture(cmdB, m_SkyViewLutTex);
	}
	d3d12::command_buffer::end_section(cmdB);
}

// Render the sky
void Sky::render_sky(CommandBuffer cmdB, RenderTexture colorBuffer, RenderTexture depthBuffer, ConstantBuffer globalCB)
{
	d3d12::command_buffer::start_section(cmdB, "Render Sky");
	{
		// Set the render target
		d3d12::command_buffer::set_render_texture(cmdB, colorBuffer, depthBuffer);

		// CBVs
		d3d12::command_buffer::set_graphics_pipeline_buffer_cbv(cmdB, m_SkyRendererFastGP, 0, globalCB);
		d3d12::command_buffer::set_graphics_pipeline_buffer_cbv(cmdB, m_SkyRendererFastGP, 1, m_SkyAtmosphereCB);

		// SRVs
		d3d12::command_buffer::set_graphics_pipeline_texture_srv(cmdB, m_SkyRendererFastGP, 0, m_TransmittanceLutTex);
		d3d12::command_buffer::set_graphics_pipeline_texture_srv(cmdB, m_SkyRendererFastGP, 1, m_MultiScatteringLutTex);
		d3d12::command_buffer::set_graphics_pipeline_texture_srv(cmdB, m_SkyRendererFastGP, 2, m_SkyViewLutTex);
		d3d12::command_buffer::set_graphics_pipeline_render_texture_srv(cmdB, m_SkyRendererFastGP, 3, depthBuffer);

		// Sampler
		SamplerDescriptor sampler({ FilterMode::Linear, SamplerMode::Clamp, SamplerMode::Clamp, SamplerMode::Clamp });
		d3d12::command_buffer::set_graphics_pipeline_sampler(cmdB, m_SkyRendererFastGP, 0, sampler);

		// Draw a full screen quad
		d3d12::command_buffer::draw_procedural(cmdB, m_SkyRendererFastGP, 1, 1);

		// CBVs
		d3d12::command_buffer::set_graphics_pipeline_buffer_cbv(cmdB, m_SkyRendererGP, 0, globalCB);
		d3d12::command_buffer::set_graphics_pipeline_buffer_cbv(cmdB, m_SkyRendererGP, 1, m_SkyAtmosphereCB);

		// SRVs
		d3d12::command_buffer::set_graphics_pipeline_texture_srv(cmdB, m_SkyRendererGP, 0, m_TransmittanceLutTex);
		d3d12::command_buffer::set_graphics_pipeline_texture_srv(cmdB, m_SkyRendererGP, 1, m_MultiScatteringLutTex);
		d3d12::command_buffer::set_graphics_pipeline_texture_srv(cmdB, m_SkyRendererGP, 2, m_SkyViewLutTex);
		d3d12::command_buffer::set_graphics_pipeline_render_texture_srv(cmdB, m_SkyRendererGP, 3, depthBuffer);

		// Sampler
		sampler = { FilterMode::Linear, SamplerMode::Clamp, SamplerMode::Clamp, SamplerMode::Clamp };
		d3d12::command_buffer::set_graphics_pipeline_sampler(cmdB, m_SkyRendererGP, 0, sampler);

		// Draw a full screen quad
		d3d12::command_buffer::draw_procedural(cmdB, m_SkyRendererGP, 1, 1);
	}
	d3d12::command_buffer::end_section(cmdB);
}
#pragma once

// Bacasable includes
#include "graphics/types.h"
#include "render_pipeline/camera.h"

class Sky
{
public:
	Sky();
	~Sky();

	// Init and Release functions
	void initialize(GraphicsDevice device);
	void release();

	// Reload the shaders
	void reload_shaders(const std::string& shaderLibrary);

	// Pre-rendering steps
	void pre_render(CommandBuffer cmd, ConstantBuffer globalCB, float planetRadius, float3 planetCenter);

	// Update the internal lookup tables
	void update_sky(CommandBuffer cmdB, ConstantBuffer globalCB, float planetRadius, float3 planetCenter);

	// Render the sky
	void render_sky(CommandBuffer cmd, RenderTexture colorBuffer, RenderTexture depthBuffer, ConstantBuffer globalCB);

	// Access the resources
	ConstantBuffer constant_buffer() const { return m_SkyAtmosphereCB; }
	Texture transmittance_lut() const { return m_TransmittanceLutTex; }
	Texture multi_scattering_lut() const { return m_MultiScatteringLutTex; }

private:
	void update_constant_buffer(CommandBuffer cmdB, float planetRadius, float3 planetCenter);

private:
	// Generic graphics resources
	GraphicsDevice m_Device = 0;

	// Buffers that hold the precomputations
	Texture m_TransmittanceLutTex = 0;
	Texture m_MultiScatteringLutTex = 0;
	Texture m_SkyViewLutTex = 0;

	// Required shaders
	ComputeShader m_TransmittanceLutCS = 0;
	ComputeShader m_MultiScatteringLutCS = 0;
	ComputeShader m_SkyViewLutCS = 0;
	GraphicsPipeline m_SkyRendererFastGP = 0;
	GraphicsPipeline m_SkyRendererGP = 0;

	// Sky constant buffer
	ConstantBuffer m_SkyAtmosphereCB = 0;
};
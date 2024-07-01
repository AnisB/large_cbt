#pragma once

// Project includes
#include "graphics/types.h"
#include "math/types.h"

// System includes
#include <string>

class WaterData
{
public:
	// Cst & Dst
	WaterData();
	~WaterData();

	// Init and release
	void initialize(GraphicsDevice device);
	void release();
	void reset_time();

	// Access the resources
	Texture get_spectrum_texture() const { return m_SpectrumTexture; }
	Texture get_displacement_texture() const { return m_DisplacementTexture; }
	Texture get_sg_texture() const { return m_SurfaceGradientTexture; }
	ConstantBuffer get_simulation_cb() const { return m_SimulationCB; }
	ConstantBuffer get_deformation_cb() const { return m_DeformationCB; }

	// Set choppiness
	void set_choppiness(float choppiness) { m_Choppiness = choppiness; }

	// Set amplification
	void set_amplification(float amplification) { m_Amplification = amplification; }

	// Set wind speed
	void set_wind_speed(float4 windSpeed) { m_WindSpeed = windSpeed; }

	// Set dir dampner
	void set_dir_dampner(float4 dirDamp) { m_DirDampner = dirDamp; }

	// Should the spectrum be re-evaluated
	bool valid_spectrum() const;
	void validate_spectrum();

	// Current time
	void update_simulation(const float currentTime);

	// Upload the constant buffers to the GPU
	void upload_constant_buffers(CommandBuffer cmd);
		
	// Render the UI
	void render_ui_global();
	void render_ui_patch();

private:
	// Runtime resources
	Texture m_SpectrumTexture = 0;
	Texture m_DisplacementTexture = 0;
	Texture m_SurfaceGradientTexture = 0;
	ConstantBuffer m_SimulationCB = 0;
	ConstantBuffer m_DeformationCB = 0;

	// Simulation state
	bool m_Initialized = false;
	float4 m_WindSpeed = { 0.0, 0.0, 0.0, 0.0 };
	float4 m_InternalWindSpeed = { 0.0, 0.0, 0.0, 0.0 };
	float4 m_DirDampner = { 0.0, 0.0, 0.0, 0.0 };
	float4 m_InternalDirDampner = { 0.0, 0.0, 0.0, 0.0 };
	bool m_PatchFlag[4] = { true, true, true,true };

	// Runtime parameters
	float m_Choppiness = 0.0;
	float m_Amplification = 0.0;
	float m_TimeMultiplier = 0.0;
	bool m_Attenuation = false;

	// Time
	float m_PreviousTime = 0.0;
	float m_AccumulatedTime = 0.0;
	bool m_ActiveSimulation = false;
};

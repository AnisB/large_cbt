#pragma once

// Project includes
#include "graphics/types.h"
#include "math/types.h"
#include "water/water_data.h"

// System includes
#include <string>

class WaterSimulation
{
public:
    WaterSimulation();
    ~WaterSimulation();

    // Initialize and release the simulation
    void initialize(GraphicsDevice graphicsDevice);
    void release();

    // reload the shaders
    void reload_shaders(const std::string& shaderLibrary);

    // Evaluate the simulation at a given time stamp
    void evaluate(CommandBuffer cmdB, WaterData& waterData) const;

private:
    // Device
    GraphicsDevice m_Device = 0;

    // Kernels
    ComputeShader m_InitSpectrumCS = 0;
    ComputeShader m_DispersionCS = 0;
    ComputeShader m_InverseFFTRowCS = 0;
    ComputeShader m_InverseFFTColCS = 0;
    ComputeShader m_EvaluateSurfaceGradientsCS = 0;
    ComputeShader m_BuildMipMapsCS = 0;

    // Resources
    Texture m_HImaginaryTexture = 0;
    Texture m_FFTRowPassRealTexture = 0;
    Texture m_FFTRowPassImaginaryTexture = 0;
};
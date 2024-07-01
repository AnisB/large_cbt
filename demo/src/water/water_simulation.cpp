// Project includes
#include "graphics/descriptors.h"
#include "graphics/dx12_backend.h"
#include "tools/security.h"

// Project includes
#include "render_pipeline/constants.h"
#include "render_pipeline/constant_buffers.h"
#include "water/water_simulation.h"
#include "tools/shader_utils.h"

#pragma region WATER_SIMULATION

// CBVs
#define WATER_SIMULATION_CB_SLOT CBV_SLOT(0)
#define WATER_SIMULATION_CBV_COUNT 1

// SRVs
#define SPECTRUM_BUFFER_SLOT SRV_SLOT(0)
#define DISPLACEMENT_BUFFER_SLOT SRV_SLOT(1)
#define WATER_SIMULATION_SRV_COUNT 2

// UAVs
#define SPECTRUM_RW_BUFFER_SLOT UAV_SLOT(0)
#define HT_REAL_RW_BUFFER_SLOT UAV_SLOT(1)
#define HT_IMAGINARY_RW_BUFFER_SLOT UAV_SLOT(2)
#define SURFACE_GRADIENT_MIP0_SLOT UAV_SLOT(3)
#define SURFACE_GRADIENT_MIP1_SLOT UAV_SLOT(4)
#define SURFACE_GRADIENT_MIP2_SLOT UAV_SLOT(5)
#define SURFACE_GRADIENT_MIP3_SLOT UAV_SLOT(6)
#define SURFACE_GRADIENT_MIP4_SLOT UAV_SLOT(7)
#define WATER_SIMULATION_UAV_COUNT 8

// File and kernels
const char* water_simulation_shader_file = "WaterSimulation.compute";
const char* initialize_spectrum_kernel = "InitializeSpectrum";
const char* dispersion_kernel = "Dispersion";
const char* evaluate_surface_gradients_kernel = "EvaluateSurfaceGradients";
const char* build_mip_maps_kernel = "BuildMipMaps";
#pragma endregion

#pragma region FOURIER_TRANSFORM
// SRVS
#define REAL_BUFFER_BINDING_SLOT SRV_SLOT(0)
#define IMAGINARY_BUFFER_BINDING_SLOT SRV_SLOT(1)
#define FOURIER_TRANSFORM_SRV_COUNT 2

// UAVs
#define REAL_RW_BUFFER_BINDING_SLOT UAV_SLOT(0)
#define IMAGINARY_RW_BUFFER_BINDING_SLOT UAV_SLOT(1)
#define FOURIER_TRANSFORM_UAV_COUNT 2

// File and kernel
const char* fourier_transform_shader_file = "FourierTransform.compute";
const char* inverse_fft_kernel = "InverseFFT";
#pragma endregion

WaterSimulation::WaterSimulation()
{
}

WaterSimulation::~WaterSimulation()
{
}

void WaterSimulation::initialize(GraphicsDevice graphicsDevice)
{
    // Keep track of the device
    m_Device = graphicsDevice;

    // Global descriptor
    TextureDescriptor descriptor;
    descriptor.type = TextureType::Tex2DArray;
    descriptor.width = g_WaterSimResolution;
    descriptor.height = g_WaterSimResolution;
    descriptor.depth = g_WaterSimBandCount;
    descriptor.mipCount = 1;
    descriptor.isUAV = true;
    descriptor.clearColor = float4({ 0.0, 0.0, 0.0, 0.0 });

    // Intermediate textures
    descriptor.format = TextureFormat::R16G16B16A16_Float;
    m_HImaginaryTexture = d3d12::graphics_resources::create_texture(graphicsDevice, descriptor);
    m_FFTRowPassRealTexture = d3d12::graphics_resources::create_texture(graphicsDevice, descriptor);
    m_FFTRowPassImaginaryTexture = d3d12::graphics_resources::create_texture(graphicsDevice, descriptor);
}

void WaterSimulation::release()
{
    // Resources
    d3d12::graphics_resources::destroy_graphics_buffer(m_FFTRowPassImaginaryTexture);
    d3d12::graphics_resources::destroy_graphics_buffer(m_FFTRowPassRealTexture);
    d3d12::graphics_resources::destroy_graphics_buffer(m_HImaginaryTexture);

    // Kernels
    d3d12::compute_shader::destroy_compute_shader(m_InverseFFTColCS);
    d3d12::compute_shader::destroy_compute_shader(m_InverseFFTRowCS);
    d3d12::compute_shader::destroy_compute_shader(m_BuildMipMapsCS);
    d3d12::compute_shader::destroy_compute_shader(m_EvaluateSurfaceGradientsCS);
    d3d12::compute_shader::destroy_compute_shader(m_DispersionCS);
    d3d12::compute_shader::destroy_compute_shader(m_InitSpectrumCS);
}

void WaterSimulation::reload_shaders(const std::string& shaderLibrary)
{
    // Create the compute shaders
    ComputeShaderDescriptor csd;
    csd.includeDirectories.push_back(shaderLibrary);
    csd.filename = shaderLibrary + "\\Water\\" + water_simulation_shader_file;
    csd.cbvCount = WATER_SIMULATION_CBV_COUNT;
    csd.srvCount = WATER_SIMULATION_SRV_COUNT;
    csd.uavCount = WATER_SIMULATION_UAV_COUNT;
    csd.samplerCount = 0;

    // Initialize spectum kernel
    csd.kernelname = initialize_spectrum_kernel;
    compile_and_replace_compute_shader(m_Device, csd, m_InitSpectrumCS);

    // Dispersion kernel
    csd.kernelname = dispersion_kernel;
    compile_and_replace_compute_shader(m_Device, csd, m_DispersionCS);

    // Evaluate normals kernel
    csd.kernelname = evaluate_surface_gradients_kernel;
    compile_and_replace_compute_shader(m_Device, csd, m_EvaluateSurfaceGradientsCS);

    // Evaluate mips kernel
    csd.kernelname = build_mip_maps_kernel;
    compile_and_replace_compute_shader(m_Device, csd, m_BuildMipMapsCS);

    // Shader file
    csd.filename = shaderLibrary + "\\Water\\" + fourier_transform_shader_file;
    csd.cbvCount = 0;
    csd.srvCount = FOURIER_TRANSFORM_SRV_COUNT;
    csd.uavCount = FOURIER_TRANSFORM_UAV_COUNT;
    csd.samplerCount = 0;

    // Inverse FFT Row kernel
    csd.kernelname = inverse_fft_kernel;
    compile_and_replace_compute_shader(m_Device, csd, m_InverseFFTRowCS);

    // Inverse FFT Col kernel
    csd.defines.clear();
    csd.defines.push_back("COLUMN_PASS");
    compile_and_replace_compute_shader(m_Device, csd, m_InverseFFTColCS);
}

void WaterSimulation::evaluate(CommandBuffer cmdB, WaterData& waterData) const
{
    d3d12::command_buffer::start_section(cmdB, "Water Simulation");
    {
        // Tile count
        const uint32_t tileC = g_WaterSimResolution / 8;
        const uint32_t tileHC = g_WaterSimResolution / 16;

        // Evaluate the spectrum if this is the first frame
        if (!waterData.valid_spectrum())
        {
            // CBVs
            d3d12::command_buffer::set_compute_shader_buffer_cbv(cmdB, m_InitSpectrumCS, WATER_SIMULATION_CB_SLOT, waterData.get_simulation_cb());
            // UAVs
            d3d12::command_buffer::set_compute_shader_texture_uav(cmdB, m_InitSpectrumCS, SPECTRUM_RW_BUFFER_SLOT, waterData.get_spectrum_texture());
            // Dispatch
            d3d12::command_buffer::dispatch(cmdB, m_InitSpectrumCS, tileC, tileC, g_WaterSimBandCount);
            // Barrier
            d3d12::command_buffer::uav_barrier_texture(cmdB, waterData.get_spectrum_texture());
            // Spectrum has been updated, all good.
        }
        waterData.validate_spectrum();

        // Evaluate the dispersion
        {
            // CBVs
            d3d12::command_buffer::set_compute_shader_buffer_cbv(cmdB, m_DispersionCS, WATER_SIMULATION_CB_SLOT, waterData.get_simulation_cb());
            // SRVs
            d3d12::command_buffer::set_compute_shader_texture_srv(cmdB, m_DispersionCS, SPECTRUM_BUFFER_SLOT, waterData.get_spectrum_texture());
            // UAVs
            d3d12::command_buffer::set_compute_shader_texture_uav(cmdB, m_DispersionCS, HT_REAL_RW_BUFFER_SLOT, waterData.get_displacement_texture());
            d3d12::command_buffer::set_compute_shader_texture_uav(cmdB, m_DispersionCS, HT_IMAGINARY_RW_BUFFER_SLOT, m_HImaginaryTexture);
            // Dispatch
            d3d12::command_buffer::dispatch(cmdB, m_DispersionCS, tileC, tileC, g_WaterSimBandCount);
            // Barrier
            d3d12::command_buffer::uav_barrier_texture(cmdB, waterData.get_displacement_texture());
        }

        // Evaluate first part of the iFFT
        {
            // SRVs
            d3d12::command_buffer::set_compute_shader_texture_srv(cmdB, m_InverseFFTRowCS, REAL_BUFFER_BINDING_SLOT, waterData.get_displacement_texture());
            d3d12::command_buffer::set_compute_shader_texture_srv(cmdB, m_InverseFFTRowCS, IMAGINARY_BUFFER_BINDING_SLOT, m_HImaginaryTexture);
            // UAVs
            d3d12::command_buffer::set_compute_shader_texture_uav(cmdB, m_InverseFFTRowCS, REAL_RW_BUFFER_BINDING_SLOT, m_FFTRowPassRealTexture);
            d3d12::command_buffer::set_compute_shader_texture_uav(cmdB, m_InverseFFTRowCS, IMAGINARY_RW_BUFFER_BINDING_SLOT, m_FFTRowPassImaginaryTexture);
            // Dispatch
            d3d12::command_buffer::dispatch(cmdB, m_InverseFFTRowCS, 1, g_WaterSimResolution, g_WaterSimBandCount);
            // Barrier
            d3d12::command_buffer::uav_barrier_texture(cmdB, m_FFTRowPassRealTexture);
        }

        // Evaluate second part of the iFFT
        {
            // SRVs
            d3d12::command_buffer::set_compute_shader_texture_srv(cmdB, m_InverseFFTColCS, REAL_BUFFER_BINDING_SLOT, m_FFTRowPassRealTexture);
            d3d12::command_buffer::set_compute_shader_texture_srv(cmdB, m_InverseFFTColCS, IMAGINARY_BUFFER_BINDING_SLOT, m_FFTRowPassImaginaryTexture);
            // UAVs
            d3d12::command_buffer::set_compute_shader_texture_uav(cmdB, m_InverseFFTColCS, REAL_RW_BUFFER_BINDING_SLOT, waterData.get_displacement_texture());
            // Dispatch
            d3d12::command_buffer::dispatch(cmdB, m_InverseFFTColCS, 1, g_WaterSimResolution, g_WaterSimBandCount);
            // Barrier
            d3d12::command_buffer::uav_barrier_texture(cmdB, waterData.get_displacement_texture());
        }

        // Evaluate water surface gradients
        {
            // CBVs
            d3d12::command_buffer::set_compute_shader_buffer_cbv(cmdB, m_EvaluateSurfaceGradientsCS, WATER_SIMULATION_CB_SLOT, waterData.get_simulation_cb());
            // SRVs
            d3d12::command_buffer::set_compute_shader_texture_srv(cmdB, m_EvaluateSurfaceGradientsCS, HT_REAL_RW_BUFFER_SLOT, waterData.get_displacement_texture());
            // UAVs
            d3d12::command_buffer::set_compute_shader_texture_uav(cmdB, m_EvaluateSurfaceGradientsCS, SURFACE_GRADIENT_MIP0_SLOT, waterData.get_sg_texture());
            // Dispatch
            d3d12::command_buffer::dispatch(cmdB, m_EvaluateSurfaceGradientsCS, tileC, tileC, g_WaterSimBandCount);
            // Barrier
            d3d12::command_buffer::uav_barrier_texture(cmdB, waterData.get_sg_texture());
        }

        // Evaluate the mip maps for the normals
        {
            // CBVs
            d3d12::command_buffer::set_compute_shader_buffer_cbv(cmdB, m_BuildMipMapsCS, WATER_SIMULATION_CB_SLOT, waterData.get_simulation_cb());
            // UAVs
            d3d12::command_buffer::set_compute_shader_texture_uav(cmdB, m_BuildMipMapsCS, SURFACE_GRADIENT_MIP0_SLOT, waterData.get_sg_texture(), 0);
            d3d12::command_buffer::set_compute_shader_texture_uav(cmdB, m_BuildMipMapsCS, SURFACE_GRADIENT_MIP1_SLOT, waterData.get_sg_texture(), 1);
            d3d12::command_buffer::set_compute_shader_texture_uav(cmdB, m_BuildMipMapsCS, SURFACE_GRADIENT_MIP2_SLOT, waterData.get_sg_texture(), 2);
            d3d12::command_buffer::set_compute_shader_texture_uav(cmdB, m_BuildMipMapsCS, SURFACE_GRADIENT_MIP3_SLOT, waterData.get_sg_texture(), 3);
            d3d12::command_buffer::set_compute_shader_texture_uav(cmdB, m_BuildMipMapsCS, SURFACE_GRADIENT_MIP4_SLOT, waterData.get_sg_texture(), 4);
            // Dispatch
            d3d12::command_buffer::dispatch(cmdB, m_BuildMipMapsCS, tileHC, tileHC, g_WaterSimBandCount);
            // Barrier
            d3d12::command_buffer::uav_barrier_texture(cmdB, waterData.get_sg_texture());
        }
    }
    d3d12::command_buffer::end_section(cmdB);
}
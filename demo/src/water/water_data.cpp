// Project includes
#include "graphics/dx12_backend.h"
#include "imgui/imgui.h"
#include "math/operators.h"
#include "render_pipeline/constants.h"
#include "render_pipeline/constant_buffers.h"
#include "water/water_data.h"

WaterData::WaterData()
{
}

WaterData::~WaterData()
{
}

// Init and release
void WaterData::initialize(GraphicsDevice device)
{
    // Global state
    m_Initialized = false;
    m_WindSpeed = { 100.0f, 70.0f, 100.0f, 30.0f };
    m_InternalWindSpeed = { 0.0, 0.0, 0.0, 0.0 };
    m_DirDampner = { 0.4, 0.2, 0.8, 1.0 };
    m_ActiveSimulation = true;
    m_Choppiness = 2.7f;
    m_Amplification = 1.2f;
    m_TimeMultiplier = 3.0f;
    m_Attenuation = true;
    m_AccumulatedTime = 0.0;

    // Create the textures
    TextureDescriptor descriptor;
    descriptor.type = TextureType::Tex2DArray;
    descriptor.width = g_WaterSimResolution;
    descriptor.height = g_WaterSimResolution;
    descriptor.depth = g_WaterSimBandCount;
    descriptor.mipCount = 1;
    descriptor.isUAV = true;
    descriptor.clearColor = float4({ 0.0, 0.0, 0.0, 0.0 });

    // Initial spectrum
    descriptor.format = TextureFormat::R16G16_Float;
    m_SpectrumTexture = d3d12::graphics_resources::create_texture(device, descriptor);

    descriptor.format = TextureFormat::R16G16B16A16_Float;
    m_DisplacementTexture = d3d12::graphics_resources::create_texture(device, descriptor);
    descriptor.mipCount = 5;
    m_SurfaceGradientTexture = d3d12::graphics_resources::create_texture(device, descriptor);

    // Constant buffer
    m_SimulationCB = d3d12::graphics_resources::create_constant_buffer(device, sizeof(WaterSimulationCB), ConstantBufferType::Mixed);
    m_DeformationCB = d3d12::graphics_resources::create_constant_buffer(device, sizeof(DeformationCB), ConstantBufferType::Mixed);
}

void WaterData::release()
{
    // Constant buffer
    d3d12::graphics_resources::destroy_constant_buffer(m_DeformationCB);
    d3d12::graphics_resources::destroy_constant_buffer(m_SimulationCB);

    // Textures
    d3d12::graphics_resources::destroy_texture(m_SurfaceGradientTexture);
    d3d12::graphics_resources::destroy_texture(m_DisplacementTexture);
    d3d12::graphics_resources::destroy_texture(m_SpectrumTexture);
}

void WaterData::reset_time()
{
    m_AccumulatedTime = 0.0;
}

void WaterData::upload_constant_buffers(CommandBuffer cmd)
{
    d3d12::command_buffer::start_section(cmd, "Update Water Constant Buffer");
    {
        // Update simulation CB
        WaterSimulationCB simCB;
        simCB._SimulationRes = g_WaterSimResolution;
        simCB._SimulationTime = m_AccumulatedTime;
        simCB._Choppiness = m_Choppiness;
        simCB._Amplification = m_Amplification;
        simCB._PatchSize = g_WaterSimPatchSize;
        simCB._PatchWindOrientation = float4({ -PI / 4.0f, PI / 2.0f, PI / 4.0f, 0 });
        simCB._PatchDirectionDampener = m_DirDampner;
        simCB._PatchWindSpeed = m_WindSpeed * g_KMPerHourToMPerSec;
        d3d12::graphics_resources::set_constant_buffer(m_SimulationCB, (const char*)&simCB, sizeof(WaterSimulationCB));
        d3d12::command_buffer::upload_constant_buffer(cmd, m_SimulationCB);

        // Update deformation CB
        DeformationCB deformationCB;
        deformationCB._PatchSize = g_WaterSimPatchSize;
        deformationCB._Choppiness = m_Choppiness;
        deformationCB._Amplification = m_Amplification;
        deformationCB._PatchRoughness = g_WaterSimPatchRoughness;
        deformationCB._Attenuation = m_Attenuation ? 1 : 0;
        deformationCB._PatchFlags = (m_PatchFlag[0] ? 0x1 : 0x0) | (m_PatchFlag[1] ? 0x2 : 0x0) | (m_PatchFlag[2] ? 0x4 : 0x0) | (m_PatchFlag[3] ? 0x8 : 0x0);
        d3d12::graphics_resources::set_constant_buffer(m_DeformationCB, (const char*)&deformationCB, sizeof(DeformationCB));
        d3d12::command_buffer::upload_constant_buffer(cmd, m_DeformationCB);
    }
    d3d12::command_buffer::end_section(cmd);
}

bool WaterData::valid_spectrum() const
{
    // Do we need to re-evalute the spectrum?
    return m_Initialized 
        && m_InternalWindSpeed.x == m_WindSpeed.x && m_InternalWindSpeed.y == m_WindSpeed.y && m_InternalWindSpeed.z == m_WindSpeed.z && m_InternalWindSpeed.w == m_WindSpeed.w
        && m_InternalDirDampner.x == m_DirDampner.x && m_InternalDirDampner.y == m_DirDampner.y && m_InternalDirDampner.z == m_DirDampner.z && m_InternalDirDampner.w == m_DirDampner.w;
}

void WaterData::validate_spectrum()
{
    // Spectrum has been evaluated, we're all good
    m_Initialized = true;
    m_InternalWindSpeed = m_WindSpeed;
    m_InternalDirDampner = m_DirDampner;
}

void WaterData::render_ui_global()
{
    ImGui::Checkbox("Water Simulation", &m_ActiveSimulation);
    ImGui::SliderFloat("Wind Speed0", &m_WindSpeed.x, 0.0f, 500.0f);
    ImGui::SliderFloat("Wind Speed1", &m_WindSpeed.y, 0.0f, 250.0f);
    ImGui::SliderFloat("Wind Speed2", &m_WindSpeed.z, 0.0f, 100.0f);
    ImGui::SliderFloat("Wind Speed3", &m_WindSpeed.w, 0.0f, 30.0f);
    ImGui::SliderFloat("Choppiness", &m_Choppiness, 0.0f, 5.0f);
    ImGui::SliderFloat("Time Multiplier", &m_TimeMultiplier, 0.0f, 5.0f);
}

void WaterData::render_ui_patch()
{
    ImGui::Checkbox("Distance Attenuation", &m_Attenuation);
    ImGui::Checkbox("Band 0", &m_PatchFlag[0]);
    ImGui::SameLine();
    ImGui::Checkbox("Band 1", &m_PatchFlag[1]);
    ImGui::SameLine();
    ImGui::Checkbox("Band 2", &m_PatchFlag[2]);
    ImGui::SameLine();
    ImGui::Checkbox("Band 3", &m_PatchFlag[3]);
}

void WaterData::update_simulation(const float currentTime)
{
    // Update the time
    float deltaTime = currentTime - m_PreviousTime;
    m_PreviousTime = currentTime;

    // Increment the time if the simulatio is active
    if (m_ActiveSimulation)
        m_AccumulatedTime += deltaTime * m_TimeMultiplier;
}
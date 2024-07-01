#pragma once

// Project includes
#include "rendering/frustum.h"
#include "cbt/leb_matrix_cache.h"
#include "cbt/cbt_utility.h"
#include "mesh/mesh_updater.h"
#include "moon/moon_deformer.h"
#include "render_pipeline/cubemap.h"
#include "render_pipeline/earth_renderer.h"
#include "render_pipeline/moon_renderer.h"
#include "render_pipeline/planet.h"
#include "render_pipeline/sky.h"
#include "render_pipeline/camera_controller.h"
#include "render_pipeline/update_properties.h"
#include "tools/mip_builder.h"
#include "tools/profiling_helper.h"
#include "water/water_deformer.h"
#include "water/water_simulation.h"

// System includes
#include <Windows.h>
#include <string>

class SpaceRenderer
{
public:
    // Cst & Dst
    SpaceRenderer();
    ~SpaceRenderer();

    // Initialization and release
    void initialize(HINSTANCE hInstance, const char* projectDirectory);
    void release();

    // Runtime loop
    void render_loop();

private:
    // Init
    void reload_shaders();
    void initialize_geometry();
    void release_geometry();
    void reset_cbt_type();

    // Rendering
    void prepare_rendering(CommandBuffer cmd);
    void render_ui(CommandBuffer cmd, RenderTexture rTexture);
    void update_constant_buffers(CommandBuffer cmd);

    // render pipelines
    void default_render_pipeline(CommandBuffer cmd, bool earthIsVisible, bool moonIsVisible, double earthDistance, double moonDistance);
    void ray_tracing_render_pipeline(CommandBuffer cmd, bool earthIsVisible, bool moonIsVisible, double earthDistance, double moonDistance);
    void render_pipeline(CommandBuffer cmd);

    // Update
    void update(double deltaTime);
    void evaluate_sun_direction();

    // Controls
    void process_key_event(uint32_t keyCode, bool state);

private:
    // Graphics Backend
    GraphicsDevice m_Device = 0;
    RenderWindow m_Window = 0;
    CommandQueue m_CmdQueue = 0;
    CommandBuffer m_CmdBuffer = 0;
    SwapChain m_SwapChain = 0;
    bool m_RayTracingSupported = false;

    // Project directory
    std::string m_ProjectDir = "";

    // Global Rendering Resources
    ConstantBuffer m_GlobalCB = 0;
    RenderTexture m_DepthBuffer = 0;
    RenderTexture m_VisibilityBuffer = 0;
    RenderTexture m_ColorTexture = 0;

    // Global rendering properties
    uint32_t m_FrameIndex = 0;
    double m_Time = 0.0;
    float4 m_ScreenSize = { 0.0, 0.0, 0.0, 0.0 };
    uint2 m_ScreenSizeI = { 0, 0 };
    bool m_RayTracingPath = false;

    // POV management
    bool m_MirrorPOV = true;
    UpdateProperties m_UpdateProperties = UpdateProperties();

    // Sun parameters
    float m_SunElevation = 0.0f;
    float m_SunRotation = 0.0f;
    float3 m_SunDirection = { 0.0, 0.0, 0.0};

    // Scene description
    Cubemap m_Cubemap = Cubemap();
    CameraController m_CameraController = CameraController();

    // UI controls
    bool m_DisplayUI = false;
    bool m_ActiveUpdate = false;
    bool m_ActiveWireFrame = false;
    bool m_EnableValidation = false;
    bool m_EnableOccupancy = false;
    float3 m_WireframeColor = { 0.6, 0.6, 0.6 };
    float m_WireframeSize = 0.5;
    uint32_t m_Occupancy = 0;

    // Profiling data
    ProfilingHelper m_ProfilingHelper = ProfilingHelper();

    // Geometry
    MeshUpdater m_MeshUpdater = MeshUpdater();
    LebMatrixCache m_LebMatrixCache = LebMatrixCache();
    CBTType m_CBTType = CBTType::Count;
    CBTType m_NewCBTType = CBTType::Count;

    // Water simulation
    WaterSimulation m_WaterSim = WaterSimulation();

    // Earth
    Planet m_EarthPlanet = Planet();
    WaterDeformer m_WaterDeformer = WaterDeformer();
    EarthRenderer m_EarthRenderer = EarthRenderer();
    WaterData m_EarthWaterData = WaterData();

    // Moon
    Planet m_MoonPlanet = Planet();
    MoonDeformer m_MoonDeformer = MoonDeformer();
    MoonRenderer m_MoonRenderer = MoonRenderer();
    MoonMaterial m_MoonMaterial = MoonMaterial();
    MipBuilder m_MipBuilder = MipBuilder();

    // Sky renderer
    Sky m_Sky = Sky();

    // Samplers
    SamplerDescriptor m_LinearClamp = { };
    SamplerDescriptor m_LinearWrap = { };

    // Ray Tracing data
    TopLevelAS m_TLAS = 0;
    BottomLevelAS m_EarthBLAS = 0;
    BottomLevelAS m_MoonBLAS = 0;
    ComputeShader m_VisibilityRT = 0;
};
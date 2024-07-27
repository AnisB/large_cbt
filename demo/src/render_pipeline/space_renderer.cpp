// Project includes
#include "graphics/dx12_backend.h"
#include "imgui/imgui_wrapper.h"
#include "imgui/imgui.h"
#include "math/operators.h"
#include "render_pipeline/constants.h"
#include "render_pipeline/constant_buffers.h"
#include "render_pipeline/space_renderer.h"
#include "tools/security.h"
#include "tools/shader_utils.h"
#include "tools/string_utilities.h"
#include "tools/texture_utils.h"

// System includes
#include <algorithm>
#include <chrono>
#include <math.h>

// Convinience
#undef min
#undef max

enum class ProfilingScopes
{
    Earth_Update = 0,
    Earth_Deformation,

    Moon_Update,
    Moon_Deformation,

    Total,
    Count
};

SpaceRenderer::SpaceRenderer()
{
}

SpaceRenderer::~SpaceRenderer()
{
}

void SpaceRenderer::initialize(HINSTANCE hInstance, const char* projectDirectory)
{
    // Keep the directories
    m_ProjectDir = projectDirectory;

    // Model library
    std::string modelLibrary = m_ProjectDir;
    modelLibrary += "\\models";

    // Texture library
    std::string textureLibrary = m_ProjectDir;
    textureLibrary += "\\textures";

    // Create the device
    m_Device = d3d12::graphics_device::create_graphics_device();
    m_RayTracingSupported = d3d12::graphics_device::ray_tracing_support(m_Device);

    // Generic other graphics resources
    m_Window = d3d12::window::create_window((uint64_t)hInstance, 1920, 1080, "Outer Space");
    m_CmdQueue = d3d12::command_queue::create_command_queue(m_Device);
    m_CmdBuffer = d3d12::command_buffer::create_command_buffer(m_Device);
    m_SwapChain = d3d12::swap_chain::create_swap_chain(m_Window, m_Device, m_CmdQueue, TextureFormat::R16G16B16A16_Float);

    // Evaluate the sizes
    uint2 screenSize;
    d3d12::window::viewport_size(m_Window, screenSize);
    m_ScreenSizeI = screenSize;
    m_ScreenSize = float4({ (float)m_ScreenSizeI.x, (float)m_ScreenSizeI.y, 1.0f / m_ScreenSizeI.x, 1.0f / m_ScreenSizeI.y });

    // Initialize imgui
    imgui_d3d12::initialize_imgui(m_Device, m_Window);

    // Other rendering resources
    m_GlobalCB = d3d12::graphics_resources::create_constant_buffer(m_Device, sizeof(GlobalCB), ConstantBufferType::Mixed);

    // Allocate the render textures
    {
        TextureDescriptor descriptor;
        descriptor.type = TextureType::Tex2D;
        descriptor.width = m_ScreenSizeI.x;
        descriptor.height = m_ScreenSizeI.y;
        descriptor.depth = 1;
        descriptor.mipCount = 1;

        // Depth Buffer
        descriptor.isUAV = false;
        descriptor.format = TextureFormat::Depth32Stencil8;
        descriptor.clearColor = float4({ 1.0f, 0.0f, 0.0f, 0.0f });
        descriptor.debugName = "Depth Buffer D32S8";
        m_DepthBuffer = d3d12::graphics_resources::create_render_texture(m_Device, descriptor);

        // Color Texture
        descriptor.isUAV = true;
        descriptor.format = TextureFormat::R16G16B16A16_Float;
        descriptor.clearColor = float4({ 0.0f, 0.0f, 0.0f, 1.0f });
        descriptor.debugName = "ColorTexture";
        m_ColorTexture = d3d12::graphics_resources::create_render_texture(m_Device, descriptor);

        // Visibility Buffer
        descriptor.format = TextureFormat::R16G16B16A16_UInt;
        descriptor.clearColor = float4({ 0.0f, 0.0f, 0.0f, 0.0f });
        descriptor.debugName = "VisibilityBuffer";
        m_VisibilityBuffer = d3d12::graphics_resources::create_render_texture(m_Device, descriptor);
    }

    // Initialize the samplers
    m_LinearClamp = { FilterMode::Linear, SamplerMode::Clamp, SamplerMode::Clamp, SamplerMode::Clamp };
    m_LinearWrap = { FilterMode::Linear, SamplerMode::Wrap, SamplerMode::Wrap, SamplerMode::Wrap };

    // Rendering properties
    m_FrameIndex = UINT32_MAX;
    m_Time = 0.0;
    m_ActiveUpdate = true;
    m_RayTracingPath = false;
    m_EnableValidation = false;
    m_EnableOccupancy = false;
    m_DisplayUI = true;
    m_MirrorPOV = true;

    // Initialize the sun parameters
    m_SunElevation = 90.0f * DEG_TO_RAD;
    m_SunRotation = 180.0f * DEG_TO_RAD;
    evaluate_sun_direction();

    // Initialize the controller
    m_CameraController.initialize(m_ScreenSizeI.x, m_ScreenSizeI.y, g_CameraFOV, m_ProjectDir);
    m_CameraController.evaluate_camera_matrices();

    // Initialize the mesh updater
    m_MeshUpdater.initialize(m_Device, m_CmdQueue);
    m_LebMatrixCache.intialize(m_Device, m_CmdQueue, m_CmdBuffer, LEB_MATRIX_CACHE_SIZE);

    // Initialize the sky
    m_Sky.initialize(m_Device);

    // Initialize the water simulation
    m_WaterSim.initialize(m_Device);

    // Initialize the cubemap
    m_Cubemap.initialize(m_Device, m_CmdQueue, m_CmdBuffer, textureLibrary);

    // Initialize the earth
    m_WaterDeformer.initialize(m_Device);
    m_EarthWaterData.initialize(m_Device);
    m_EarthRenderer.initialize(m_Device);

    // Initialize the moon
    m_MoonDeformer.initialize(m_Device, textureLibrary.c_str());
    m_MoonMaterial.initialize(m_Device, m_CmdQueue, m_CmdBuffer, textureLibrary);
    m_MoonRenderer.initialize(m_Device, textureLibrary);
    m_MipBuilder.initialize(m_Device);

    // Profiling helper
    m_ProfilingHelper.initialize(m_Device, m_CmdQueue, (uint32_t)ProfilingScopes::Count);

    // Initialize the geometry
    m_CBTType = CBTType::OCBT_128K;
    m_NewCBTType = m_CBTType;
    initialize_geometry();

    // Load all the shaders
    reload_shaders();
}

void SpaceRenderer::reload_shaders()
{
    // Location of the shader library
    std::string shaderLibrary = m_ProjectDir;
    shaderLibrary += "\\shaders";

    // Ray Tracing pass
    if (m_RayTracingSupported)
    {
        // Create the compute shaders
        ComputeShaderDescriptor csd;
        csd.includeDirectories.push_back(shaderLibrary);
        csd.filename = shaderLibrary + "\\Visibility\\VisibilityPassRT.compute";
        csd.srvCount = 1;
        csd.uavCount = 1;
        csd.cbvCount = 1;

        // Reset kernel
        csd.kernelname = "VisibilityPass";
        compile_and_replace_compute_shader(m_Device, csd, m_VisibilityRT);
    }

    // Earth related
    m_EarthPlanet.reload_shaders(shaderLibrary);
    m_WaterDeformer.reload_shaders(shaderLibrary);
    m_EarthRenderer.reload_shaders(shaderLibrary);

    // Moon related
    m_MoonPlanet.reload_shaders(shaderLibrary);
    m_MoonDeformer.reload_shaders(shaderLibrary);
    m_MoonRenderer.reload_shaders(shaderLibrary);
    m_MoonMaterial.reload_shaders(shaderLibrary);

    // Individual components
    m_WaterSim.reload_shaders(shaderLibrary);
    m_Cubemap.reload_shaders(shaderLibrary);
    m_MeshUpdater.reload_shaders(shaderLibrary, m_CBTType);
    m_Sky.reload_shaders(shaderLibrary);
    m_MipBuilder.reload_shaders(shaderLibrary);
}

void SpaceRenderer::initialize_geometry()
{
    // Num elements the CBT holds
    const uint32_t cbtNumElements = cbt_num_elements(m_CBTType);
    CBT* cbt = create_cbt(m_CBTType);

    // Load the planet model
    std::string planetMeshPath = m_ProjectDir + "\\models\\icosahedron.ccm";
    CPUMesh planetCPUMesh;
    load_cpu_mesh(planetMeshPath.c_str(), cbtNumElements, planetCPUMesh);

    // Initialize the earth
    m_EarthPlanet.initialize(m_Device, m_CmdQueue, m_CmdBuffer, g_EarthRadius, g_EarthCenter, g_EarthImpostorToggle, g_EarthTriangleSize, EARTH_MATERIAL, planetCPUMesh, *cbt);

    // Initialize the moon
    m_MoonPlanet.initialize(m_Device, m_CmdQueue, m_CmdBuffer, g_MoonRadius, g_MoonCenter, g_MoonImpostorToggle, g_MoonTriangleSize, MOON_MATERIAL, planetCPUMesh, *cbt);

    // delete the CBT instance
    delete cbt;

    // Create the acceleration structures
    if (m_RayTracingSupported)
    {
        // Earth mesh
        const CBTMesh& earthMesh = m_EarthPlanet.get_cbt_mesh();
        const BaseMesh& earthBaseMesh = m_EarthPlanet.get_base_mesh();
        m_EarthBLAS = d3d12::graphics_resources::create_blas(m_Device, earthMesh.currentVertexBuffer, earthMesh.totalNumElements * 3, earthBaseMesh.indexBuffer, earthMesh.totalNumElements);

        // Moon mesh
        const CBTMesh& moonMesh = m_MoonPlanet.get_cbt_mesh();
        const BaseMesh& moonBaseMesh = m_MoonPlanet.get_base_mesh();
        m_MoonBLAS = d3d12::graphics_resources::create_blas(m_Device, moonMesh.currentVertexBuffer, moonMesh.totalNumElements * 3, moonBaseMesh.indexBuffer, moonMesh.totalNumElements);

        // Create the tlas
        m_TLAS = d3d12::graphics_resources::create_tlas(m_Device, 2);
        d3d12::graphics_resources::set_tlas_instance(m_TLAS, m_EarthBLAS, 0);
        d3d12::graphics_resources::set_tlas_instance(m_TLAS, m_MoonBLAS, 1);
        d3d12::graphics_resources::upload_tlas_instance_data(m_TLAS);
    }
}
void SpaceRenderer::release_geometry()
{
    m_EarthPlanet.release();
    m_MoonPlanet.release();

    // Destroy the acceleration structures
    if (m_RayTracingSupported)
    {
        d3d12::graphics_resources::destroy_blas(m_EarthBLAS);
        d3d12::graphics_resources::destroy_blas(m_MoonBLAS);
        d3d12::graphics_resources::destroy_tlas(m_TLAS);
    }
}

void SpaceRenderer::reset_cbt_type()
{
    release_geometry();
    m_CBTType = m_NewCBTType;
    initialize_geometry();

    // Location of the shader library
    std::string shaderLibrary = m_ProjectDir;
    shaderLibrary += "\\shaders";
    m_MeshUpdater.reload_shaders(shaderLibrary, m_CBTType);
    m_EarthPlanet.reload_shaders(shaderLibrary);
    m_MoonPlanet.reload_shaders(shaderLibrary);

    prepare_rendering(m_CmdBuffer);
}

void SpaceRenderer::release()
{
    // Destroy the profiling helper
    m_ProfilingHelper.release();

    // Destroy the acceleration structures
    release_geometry();

    // Moon cleanup
    m_MipBuilder.release();
    m_MoonRenderer.release();
    m_MoonMaterial.release();
    m_MoonDeformer.release();

    // Earth cleanup
    m_EarthWaterData.release();
    m_EarthRenderer.release();
    m_WaterDeformer.release();

    // Cubemap cleaup
    m_Cubemap.release();

    // Water Sim cleanup
    m_WaterSim.release();

    // Sky cleanup
    m_Sky.release();

    // Mesh updater clean up
    m_MeshUpdater.release();
    m_LebMatrixCache.release();

    // Release the RT shader
    if (m_RayTracingSupported)
        d3d12::compute_shader::destroy_compute_shader(m_VisibilityRT);

    // Imgui cleanup
    imgui_d3d12::release_imgui();

    // Other rendering resources
    d3d12::graphics_resources::destroy_render_texture(m_VisibilityBuffer);
    d3d12::graphics_resources::destroy_render_texture(m_ColorTexture);
    d3d12::graphics_resources::destroy_render_texture(m_DepthBuffer);
    d3d12::graphics_resources::destroy_constant_buffer(m_GlobalCB);

    // Generic graphics api cleanup
    d3d12::swap_chain::destroy_swap_chain(m_SwapChain);
    d3d12::command_buffer::destroy_command_buffer(m_CmdBuffer);
    d3d12::command_queue::destroy_command_queue(m_CmdQueue);
    d3d12::window::destroy_window(m_Window);

    // Release the device
    d3d12::graphics_device::destroy_graphics_device(m_Device);
}

void SpaceRenderer::render_ui(CommandBuffer cmd, RenderTexture rTexture)
{
    if (!m_DisplayUI)
        return;

    d3d12::command_buffer::start_section(cmd, "Render UI");
    {
        // Start enqueing commands
        imgui_d3d12::start_frame();

        // Debug Params
        ImGui::SetNextWindowSize(ImVec2(225.0f, 400.0f));
        ImGui::Begin("Debug parameters");
        {
            ImGui::SeparatorText("Wireframe");
            ImGui::Checkbox("Enable", &m_ActiveWireFrame);
            if (m_ActiveWireFrame)
            {
                ImGui::InputFloat3("Color", &m_WireframeColor.x);
                ImGui::InputFloat("Size", &m_WireframeSize);
            }

            ImGui::SeparatorText("CBT");
            ImGui::Checkbox("Update CBT", &m_ActiveUpdate);
            ImGui::Checkbox("Enable Validation", &m_EnableValidation);
            ImGui::Checkbox("Enable Occupancy", &m_EnableOccupancy);
            if (m_EnableOccupancy)
            {
                std::string occupancy = "Occupancy ";
                occupancy += std::to_string(m_Occupancy);
                ImGui::Text(occupancy.c_str());
            }
            ImGui::Checkbox("Miror VP", &m_MirrorPOV);
            ImGui::SeparatorText("Other");
            if (m_RayTracingSupported)
                ImGui::Checkbox("Ray Tracing Path", &m_RayTracingPath);
            else
                ImGui::Text("Ray Tracing not supported.");

            std::string distanceToCenter = "Elevation (km) ";
            distanceToCenter += to_string_with_precision((m_CameraController.distance_to_earth_center() - g_EarthRadius) / 1000.0, 2);
            ImGui::Text(distanceToCenter.c_str());

            // Timings
            ImGui::SeparatorText("Timings");
            uint64_t earthUpdate = m_ProfilingHelper.get_scope_last_duration((uint32_t)ProfilingScopes::Earth_Update);
            uint64_t earthDeformation = m_ProfilingHelper.get_scope_last_duration((uint32_t)ProfilingScopes::Earth_Deformation);
            uint64_t moonUpdate = m_ProfilingHelper.get_scope_last_duration((uint32_t)ProfilingScopes::Moon_Update);
            uint64_t moonDeformation = m_ProfilingHelper.get_scope_last_duration((uint32_t)ProfilingScopes::Moon_Deformation);
            uint64_t total = m_ProfilingHelper.get_scope_last_duration((uint32_t)ProfilingScopes::Total);

            // Earth Update
            std::string earthUpdateD = "Earth Update ";
            earthUpdateD += std::to_string(earthUpdate) +  "(us)";
            ImGui::Text(earthUpdateD.c_str());

            std::string earthDefD = "Earth Deformation ";
            earthDefD += std::to_string(earthDeformation) + "(us)";
            ImGui::Text(earthDefD.c_str());

            // Moon
            std::string moonUpdateD = "Moon Update ";
            moonUpdateD += std::to_string(moonUpdate) + "(us)";
            ImGui::Text(moonUpdateD.c_str());

            std::string moonDefD = "Moon Deformation ";
            moonDefD += std::to_string(moonDeformation) + "(us)";
            ImGui::Text(moonDefD.c_str());

            // Max duration
            std::string totalDuration = "Total Duration ";
            totalDuration += std::to_string(total);
            totalDuration += "(us)";
            ImGui::Text(totalDuration.c_str());

            // Max duration
            std::string fpsS = "  FPS ";
            fpsS += std::to_string(1e6 / total);
            ImGui::Text(fpsS.c_str());
        }
        ImGui::End();

        // Earth parameters
        ImGui::SetNextWindowSize(ImVec2(450.0f, 525.0f));
        ImGui::Begin("Scene Parameters");
        {
            ImGui::SeparatorText("General");
            ImGui::SliderFloat("Sun Elevation", &m_SunElevation, 0, 180.0 * DEG_TO_RAD);
            ImGui::SliderFloat("Sun Rotation", &m_SunRotation, 0.0f, 360.0f * DEG_TO_RAD);

            // Previous mode
            const char* currentCBTType = g_CBTTypesNames[(uint32_t)m_CBTType];
            if (ImGui::BeginCombo("CBT Type", currentCBTType))
            {
                for (int n = 0; n < (uint32_t)CBTType::Count; n++)
                {
                    bool is_selected = (currentCBTType == g_CBTTypesNames[n]); // You can store your selection however you want, outside or inside your objects
                    if (ImGui::Selectable(g_CBTTypesNames[n], is_selected))
                    {
                        m_NewCBTType = (CBTType)n;
                        if (is_selected)
                            ImGui::SetItemDefaultFocus();   // You may set the initial focus when opening the combo (scrolling + for keyboard navigation support)
                    }
                }
                ImGui::EndCombo();
            }

            ImGui::SeparatorText("Earth");
            m_EarthPlanet.render_ui("Earth");
            m_EarthWaterData.render_ui_global();

            ImGui::SeparatorText("Moon");
            m_MoonPlanet.render_ui("Moon");
            m_MoonMaterial.render_ui();
        }
        ImGui::End();

        // Camera controller
        m_CameraController.render_ui();

        // End enqueing commands
        imgui_d3d12::end_frame();

        // Enqueue into the command buffer
        imgui_d3d12::draw_frame(cmd, rTexture);
    }
    d3d12::command_buffer::end_section(cmd);
}

void SpaceRenderer::update_constant_buffers(CommandBuffer cmd)
{
    d3d12::command_buffer::start_section(cmd, "Update Constant Buffers");
    {
        // Evaluate the sun direction
        evaluate_sun_direction();

        // Grab the camera
        const Camera& camera = m_CameraController.get_camera();

        // Global constant buffer
        GlobalCB globalCB;
        globalCB._ViewProjectionMatrix = camera.viewProjection;
        globalCB._ViewProjectionMatrixDB = camera.viewProjectionDB;
        globalCB._InvViewProjectionMatrix = camera.invViewProjection;
        globalCB._CameraPosition = camera.position;
        globalCB._CameraPositionSP = float3({ (float)camera.position.x, (float)camera.position.y, (float)camera.position.z });
        globalCB._SunDirection = m_SunDirection;
        globalCB._FrameIndex = m_FrameIndex;
        globalCB._Time = (float)m_Time;
        globalCB._ScreenSize = m_ScreenSize;
        globalCB._FarPlaneDistance = camera.nearFar.y;
        globalCB._WireFrameColor = m_WireframeColor;
        globalCB._WireFrameSize = m_WireframeSize;
        globalCB._ScreenSpaceShadow = m_RayTracingPath ? 1.0f : 0.0f;
        d3d12::graphics_resources::set_constant_buffer(m_GlobalCB, (const char*)&globalCB, sizeof(GlobalCB));
        d3d12::command_buffer::upload_constant_buffer(cmd, m_GlobalCB);

        // Update the planet constant buffers
        m_EarthPlanet.update_constant_buffers(cmd, m_UpdateProperties);
        m_MoonPlanet.update_constant_buffers(cmd, m_UpdateProperties);
        m_MoonMaterial.update_constant_buffers(cmd);

        // Update the water simulation CB
        m_EarthWaterData.update_simulation((float)m_Time);
        m_EarthWaterData.upload_constant_buffers(cmd);
    }
    d3d12::command_buffer::end_section(cmd);
}

void SpaceRenderer::process_key_event(uint32_t keyCode, bool state)
{
    switch (keyCode)
    {
        case 0x74: // F5
            if (state)
                reload_shaders();
        break;
        case 0x7A: // F11
            if (state)
                m_DisplayUI = !m_DisplayUI;
        break;
    }

    // Propagate to the camera controller
    m_CameraController.process_key_event(keyCode, state);
}

void SpaceRenderer::prepare_rendering(CommandBuffer cmd)
{
    // Grab the camera
    const Camera& camera = m_CameraController.get_camera();

    // Reset the time and frame index
    m_FrameIndex = UINT32_MAX;
    m_Time = 0.0;

    // Reset the command buffer
    d3d12::command_buffer::reset(cmd);

    // Update the static constant buffers
    m_EarthPlanet.upload_static_constant_buffers(cmd);
    m_MoonPlanet.upload_static_constant_buffers(cmd);

    // Update the runtime constant buffers
    update_constant_buffers(cmd);

    // Prepare the moon material
    m_MoonMaterial.prepare_rendering(cmd, m_MoonPlanet.get_planet_cb());
    m_MipBuilder.build_mips(cmd, m_MoonMaterial.get_albedo_texture());
    m_MipBuilder.build_mips(cmd, m_MoonMaterial.get_elevation_texture());
    m_MipBuilder.build_mips(cmd, m_MoonMaterial.get_elevation_sg_texture());
    m_MipBuilder.build_mips(cmd, m_MoonMaterial.get_detail_texture());
    m_MipBuilder.build_mips(cmd, m_MoonMaterial.get_detail_sg_texture());

    // Evaluate the water simulation for the earth
    m_WaterSim.evaluate(cmd, m_EarthWaterData);

    // Pre rendering steps of the sky
    m_Sky.pre_render(cmd, m_GlobalCB, g_EarthRadius, { (float)g_EarthCenter.x, (float)g_EarthCenter.y, (float)g_EarthCenter.z });

    // Prepare the earth's geometry
    m_MeshUpdater.reset_buffers(cmd, m_EarthPlanet.get_cbt_mesh());
    m_MeshUpdater.prepare_indirection(cmd, m_EarthPlanet.get_cbt_mesh(), m_EarthPlanet.get_geometry_cb());
    m_EarthPlanet.evaluate_leb(cmd, camera, m_GlobalCB, m_LebMatrixCache.get_leb_matrix_buffer(), true, true);
    m_WaterDeformer.apply_deformation(cmd, m_EarthPlanet.get_cbt_mesh(), m_EarthWaterData, m_GlobalCB, m_EarthPlanet.get_geometry_cb(), m_EarthPlanet.get_planet_cb(), m_EarthPlanet.get_update_cb());

    // Prepare the moon's geometry
    m_MeshUpdater.reset_buffers(cmd, m_MoonPlanet.get_cbt_mesh());
    m_MeshUpdater.prepare_indirection(cmd, m_MoonPlanet.get_cbt_mesh(), m_MoonPlanet.get_geometry_cb());
    m_MoonPlanet.evaluate_leb(cmd, camera, m_GlobalCB, m_LebMatrixCache.get_leb_matrix_buffer(), true, true);
    m_MoonDeformer.apply_deformation(cmd, m_MoonMaterial, m_GlobalCB, m_MoonPlanet.get_cbt_mesh(), m_MoonPlanet.get_geometry_cb(), m_MoonPlanet.get_planet_cb());

    // Compute an initial blas version of all meshes
    if (m_RayTracingSupported)
    {
        d3d12::command_buffer::build_blas(cmd, m_EarthBLAS);
        d3d12::command_buffer::build_blas(cmd, m_MoonBLAS);
    }

    // Close and flush the command buffer
    d3d12::command_buffer::close(cmd);
    d3d12::command_queue::execute_command_buffer(m_CmdQueue, cmd);
    d3d12::command_queue::flush(m_CmdQueue);
}

void SpaceRenderer::default_render_pipeline(CommandBuffer cmd, bool earthIsVisible, bool moonIsVisible, double earthDistance, double moonDistance)
{
    // Draw the planet (if close)
    if (earthIsVisible)
    {
        if (!m_ActiveWireFrame)
        {
            m_EarthRenderer.render_visibility_buffer(cmd, m_VisibilityBuffer, m_DepthBuffer, m_EarthPlanet, m_GlobalCB);
            m_EarthRenderer.render_material(cmd, m_ColorTexture, m_VisibilityBuffer, m_DepthBuffer, m_EarthPlanet, m_EarthWaterData.get_sg_texture(), m_Sky, m_GlobalCB, m_EarthWaterData.get_deformation_cb());
        }
        else
            m_EarthRenderer.render_solid_wire(cmd, m_ColorTexture, m_DepthBuffer, m_EarthPlanet, m_EarthWaterData.get_sg_texture(), m_Sky, m_GlobalCB, m_EarthWaterData.get_deformation_cb());
    }

    // Is the moon visible?
    if (moonIsVisible)
    {
        if (!m_ActiveWireFrame)
        {
            m_MoonRenderer.render_visibility_buffer(cmd, m_VisibilityBuffer, m_DepthBuffer, m_MoonPlanet, m_GlobalCB);
            m_MoonRenderer.render_material(cmd, m_ColorTexture, m_VisibilityBuffer, m_DepthBuffer, m_MoonMaterial, m_MoonPlanet, m_GlobalCB);
        }
        else
            m_MoonRenderer.render_solid_wire(cmd, m_ColorTexture, m_DepthBuffer, m_MoonMaterial, m_MoonPlanet, m_GlobalCB);
    }

    // The the render viewport
    d3d12::command_buffer::set_render_texture(cmd, m_ColorTexture, m_DepthBuffer);

    // Render the impostors if required
    if (earthDistance < moonDistance)
    {
        // Draw the planet (if distant)
        if (!earthIsVisible) m_EarthRenderer.render_planet_impostor(cmd, m_EarthPlanet, m_EarthWaterData, m_Sky, m_GlobalCB);
        if (!moonIsVisible) m_MoonRenderer.render_planet_impostor(cmd, m_MoonMaterial, m_MoonPlanet, m_GlobalCB, false);
    }
    else
    {
        // Draw the planet (if distant)
        if (!moonIsVisible) m_MoonRenderer.render_planet_impostor(cmd, m_MoonMaterial, m_MoonPlanet, m_GlobalCB, true);
        if (!earthIsVisible) m_EarthRenderer.render_planet_impostor(cmd, m_EarthPlanet, m_EarthWaterData, m_Sky, m_GlobalCB);
    }

    // Render the sky above
    m_Sky.render_sky(cmd, m_ColorTexture, m_DepthBuffer, m_GlobalCB);
}

void SpaceRenderer::ray_tracing_render_pipeline(CommandBuffer cmd, bool earthIsVisible, bool moonIsVisible, double earthDistance, double moonDistance)
{
    d3d12::command_buffer::start_section(cmd, "Build RTAS");
    {
        // Build the ray tracing acceleration structures
        if (earthIsVisible)
            d3d12::command_buffer::build_blas(cmd, m_EarthBLAS);
        if (moonIsVisible)
            d3d12::command_buffer::build_blas(cmd, m_MoonBLAS);
        d3d12::command_buffer::build_tlas(cmd, m_TLAS);
    }
    d3d12::command_buffer::end_section(cmd);

    d3d12::command_buffer::start_section(cmd, "Trace Scene");
    {
        // Render the scene to the visibility buffer
        d3d12::command_buffer::set_compute_shader_buffer_cbv(cmd, m_VisibilityRT, 0, m_GlobalCB);
        d3d12::command_buffer::set_compute_shader_rtas_srv(cmd, m_VisibilityRT, 0, m_TLAS);
        d3d12::command_buffer::set_compute_shader_render_texture_uav(cmd, m_VisibilityRT, 0, m_VisibilityBuffer);
        d3d12::command_buffer::dispatch(cmd, m_VisibilityRT, (m_ScreenSizeI.x + 7) / 8, (m_ScreenSizeI.y + 7) / 8, 1);
        d3d12::command_buffer::uav_barrier_buffer(cmd, m_VisibilityBuffer);
    }
    d3d12::command_buffer::end_section(cmd);

    // Render the earth if visible
    if (earthIsVisible)
        m_EarthRenderer.render_material(cmd, m_ColorTexture, m_VisibilityBuffer, m_DepthBuffer, m_EarthPlanet, m_EarthWaterData.get_sg_texture(), m_Sky, m_GlobalCB, m_EarthWaterData.get_deformation_cb());
    
    // Render the moon if visible
    if (moonIsVisible)
        m_MoonRenderer.render_material(cmd, m_ColorTexture, m_VisibilityBuffer, m_DepthBuffer, m_MoonMaterial, m_MoonPlanet, m_GlobalCB);

    // The the render viewport
    d3d12::command_buffer::set_render_texture(cmd, m_ColorTexture, m_DepthBuffer);

    // Render the impostors if required
    if (earthDistance < moonDistance)
    {
        // Draw the planet (if distant)
        if (!earthIsVisible) m_EarthRenderer.render_planet_impostor(cmd, m_EarthPlanet, m_EarthWaterData, m_Sky, m_GlobalCB);
        if (!moonIsVisible) m_MoonRenderer.render_planet_impostor(cmd, m_MoonMaterial, m_MoonPlanet, m_GlobalCB, false);
    }
    else
    {
        // Draw the planet (if distant)
        if (!moonIsVisible) m_MoonRenderer.render_planet_impostor(cmd, m_MoonMaterial, m_MoonPlanet, m_GlobalCB, true);
        if (!earthIsVisible) m_EarthRenderer.render_planet_impostor(cmd, m_EarthPlanet, m_EarthWaterData, m_Sky, m_GlobalCB);
    }
}

void SpaceRenderer::render_pipeline(CommandBuffer cmd)
{
    // Grab the camera
    const Camera& camera = m_CameraController.get_camera();

    // Reset the command buffer
    d3d12::command_buffer::reset(cmd);

    // Start the profiling
    m_ProfilingHelper.start_profiling(cmd, (uint32_t)ProfilingScopes::Total);

    // Update the constant buffers
    update_constant_buffers(cmd);

    // Update the sky
    m_Sky.update_sky(cmd, m_GlobalCB, g_EarthRadius, { (float)g_EarthCenter.x, (float)g_EarthCenter.y, (float)g_EarthCenter.z });

    // How far is the earth
    double earthDistance;
    bool earthIsVisible, earthIsUpdatable;
    m_EarthPlanet.planet_visibility(camera, earthDistance, earthIsVisible, earthIsUpdatable);

    // How far is the moon
    double moonDistance;
    bool moonIsVisible, moonIsUpdatable;
    m_MoonPlanet.planet_visibility(camera, moonDistance, moonIsVisible, moonIsUpdatable);

    // Update the earth water simulation only if we are close enought
    if (earthIsVisible)
    {
        m_EarthWaterData.update_simulation((float)m_Time);
        m_EarthWaterData.upload_constant_buffers(cmd);
        m_WaterSim.evaluate(cmd, m_EarthWaterData);
    }

    // Update the earth
    m_ProfilingHelper.start_profiling(cmd, (uint32_t)ProfilingScopes::Earth_Update);
    if (earthIsUpdatable || m_RayTracingPath)
    {
        if (m_ActiveUpdate)
            m_MeshUpdater.update(cmd, m_EarthPlanet.get_cbt_mesh(),m_GlobalCB, m_EarthPlanet.get_geometry_cb(), m_EarthPlanet.get_update_cb());
        m_EarthPlanet.evaluate_leb(cmd, camera, m_GlobalCB, m_LebMatrixCache.get_leb_matrix_buffer(), m_RayTracingPath);
    }
    m_ProfilingHelper.end_profiling(cmd, (uint32_t)ProfilingScopes::Earth_Update);

    // Update the moon
    m_ProfilingHelper.start_profiling(cmd, (uint32_t)ProfilingScopes::Moon_Update);
    if (moonIsUpdatable)
    {
        if (m_ActiveUpdate)
            m_MeshUpdater.update(cmd, m_MoonPlanet.get_cbt_mesh(), m_GlobalCB, m_MoonPlanet.get_geometry_cb(), m_MoonPlanet.get_update_cb());
        m_MoonPlanet.evaluate_leb(cmd, camera, m_GlobalCB, m_LebMatrixCache.get_leb_matrix_buffer(), m_RayTracingPath);
    }
    m_ProfilingHelper.end_profiling(cmd, (uint32_t)ProfilingScopes::Moon_Update);

    // Enable deformation
    m_ProfilingHelper.start_profiling(cmd, (uint32_t)ProfilingScopes::Earth_Deformation);
    if (earthIsUpdatable)
        m_WaterDeformer.apply_deformation(cmd, m_EarthPlanet.get_cbt_mesh(), m_EarthWaterData, m_GlobalCB, m_EarthPlanet.get_geometry_cb(), m_EarthPlanet.get_planet_cb(), m_EarthPlanet.get_update_cb());
    m_ProfilingHelper.end_profiling(cmd, (uint32_t)ProfilingScopes::Earth_Deformation);

    m_ProfilingHelper.start_profiling(cmd, (uint32_t)ProfilingScopes::Moon_Deformation);
    if (moonIsUpdatable)
        m_MoonDeformer.apply_deformation(cmd, m_MoonMaterial, m_GlobalCB, m_MoonPlanet.get_cbt_mesh(), m_MoonPlanet.get_geometry_cb(), m_MoonPlanet.get_planet_cb());
    m_ProfilingHelper.end_profiling(cmd, (uint32_t)ProfilingScopes::Moon_Deformation);

    // Validation required?
    if (m_EnableValidation)
    {
        m_MeshUpdater.validate(cmd, m_EarthPlanet.get_cbt_mesh(), m_EarthPlanet.get_geometry_cb());
        m_MeshUpdater.validate(cmd, m_MoonPlanet.get_cbt_mesh(), m_MoonPlanet.get_geometry_cb());
    }

    {
        d3d12::command_buffer::start_section(cmd, "Clear RTs");
        d3d12::command_buffer::clear_depth_stencil_texture(cmd, m_DepthBuffer, 1.0f, 0);
        d3d12::command_buffer::clear_render_texture(cmd, m_ColorTexture, float4({ 0.0, 0.0, 0.0, 1.0 }));
        d3d12::command_buffer::clear_render_texture(cmd, m_VisibilityBuffer, float4({ 0.0, 0.0, 0.0, 0.0 }));
        d3d12::command_buffer::end_section(cmd);
    }

    // Query the occupancy
    if (m_EnableOccupancy)
        m_MeshUpdater.query_occupancy(cmd, m_EarthPlanet.get_cbt_mesh());

    // The the render viewport
    d3d12::command_buffer::set_render_texture(cmd, m_ColorTexture, m_DepthBuffer);

    // The the render viewport
    d3d12::command_buffer::set_viewport(cmd, 0, 0, (uint32_t)m_ScreenSize.x, (uint32_t)m_ScreenSize.y);

    // Draw the cubemap
    m_Cubemap.render_cubemap(cmd, m_GlobalCB);

    // Render the scene
    if (!m_RayTracingPath)
        default_render_pipeline(cmd, earthIsVisible, moonIsVisible, earthDistance, moonDistance);
    else
        ray_tracing_render_pipeline(cmd, earthIsVisible, moonIsVisible, earthDistance, moonDistance);

    // Start the profiling
    m_ProfilingHelper.end_profiling(cmd, (uint32_t)ProfilingScopes::Total);

    // Render the UI
    render_ui(cmd, m_ColorTexture);

    // Grab the current swap chain render target
    RenderTexture colorBuffer = d3d12::swap_chain::get_current_render_texture(m_SwapChain);

    // Copy our texture to the swap chain RT
    d3d12::command_buffer::copy_render_texture(cmd, m_ColorTexture, colorBuffer);

    // Set the render target in present mode
    d3d12::command_buffer::transition_to_present(cmd, colorBuffer);

    // Close the command buffer
    d3d12::command_buffer::close(cmd);

    // Execute the command buffer in the command queue
    d3d12::command_queue::execute_command_buffer(m_CmdQueue, cmd);

    // Present
    d3d12::swap_chain::present(m_SwapChain);

    // Flush the queue
    d3d12::command_queue::flush(m_CmdQueue);

    // Evaluate the timing
    m_ProfilingHelper.process_scopes(m_CmdQueue);

    // Validation
    if (m_EnableValidation)
        assert_msg(m_MeshUpdater.check_if_valid(), "Validation failed.");

    // Grab the CBT's occupancy
    if (m_EnableOccupancy)
        m_Occupancy = m_MeshUpdater.get_occupancy();

    if (m_NewCBTType != m_CBTType)
        reset_cbt_type();
}

void SpaceRenderer::evaluate_sun_direction()
{
    float sinPhi = sinf(m_SunRotation);
    float cosPhi = cosf(m_SunRotation);
    float sinTheta = sinf(m_SunElevation);
    float cosTheta = cosf(m_SunElevation);
    m_SunDirection = normalize(float3({ sinTheta * sinPhi, cosTheta, sinTheta * cosPhi }));
}

void SpaceRenderer::render_loop()
{
    // Show the window
    d3d12::window::show(m_Window);
    
    // All required initializations before the render loop
    prepare_rendering(m_CmdBuffer);

    // Render loop
    bool activeLoop = true;
    m_FrameIndex = 0;
    m_Time = 0.0;
    while (activeLoop)
    {
        auto start = std::chrono::high_resolution_clock::now();

        // Handle the messages
        d3d12::window::handle_messages(m_Window);
        uint2 windowCenter = d3d12::window::window_center(m_Window);

        bool resetCursorToCenter = false;
        // Process the events
        EventData eventData;
        while (event_collector::peek_event(eventData))
        {
            switch (eventData.type)
            {
                case FrameEvent::Raw:
                    imgui_d3d12::handle_input(m_Window, eventData);
                break;
                case FrameEvent::MouseMovement:
                    resetCursorToCenter |= m_CameraController.process_mouse_movement({ (int)eventData.data0, (int)eventData.data1 }, windowCenter, m_ScreenSize);
                break;
                case FrameEvent::MouseWheel:
                    m_CameraController.process_mouse_wheel((int)eventData.data0);
                break;
                case FrameEvent::MouseButton:
                    resetCursorToCenter |= m_CameraController.process_mouse_button((MouseButton)eventData.data0, eventData.data1 != 0);
                break;
                case FrameEvent::KeyDown:
                    process_key_event(eventData.data0, true);
                break;
                case FrameEvent::KeyUp:
                    process_key_event(eventData.data0, false);
                break;
                case FrameEvent::Close:
                case FrameEvent::Destroy:
                    activeLoop = false;
                break;
            }
        }

        // Reset the cursor to the center if requested
        if (resetCursorToCenter)
            d3d12::window::set_cursor_pos(m_Window, windowCenter);

        // If there is a frame to draw
        if (event_collector::active_draw_request())
        {
            // Render the frame
            render_pipeline(m_CmdBuffer);

            // For the first frame of the play, reset the time
            if (m_CameraController.new_frame())
                m_EarthWaterData.reset_time();

            // Mark the rendering as done
            event_collector::draw_done();
        }

        // Evaluate the delta time
        auto stop = std::chrono::high_resolution_clock::now();
        std::chrono::microseconds duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
        double deltaTime = duration.count() / 1e6;

        // Add to the time
        m_Time += deltaTime;

        // Update the scene
        update(deltaTime);
    }
}

void SpaceRenderer::update(double deltaTime)
{
    // Update the controller
    m_CameraController.update(deltaTime);

    // Update the data if needed
    if (m_MirrorPOV)
    {
        const Camera& camera = m_CameraController.get_camera();
        mirror_update_properties(camera, m_UpdateProperties);
    }
}

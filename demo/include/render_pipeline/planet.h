#pragma once

// Project includes

// Project includes
#include "graphics/types.h"
#include "mesh/mesh.h"
#include "render_pipeline/camera.h"
#include "render_pipeline/update_properties.h"
#include "math/types.h"
#include "mesh/mesh_updater.h"
#include "water/water_simulation.h"

class Planet
{
public:
    // Cst & Dst
    Planet();
    ~Planet();

    // Init and release
    void initialize(GraphicsDevice device, CommandQueue cmdQ, CommandBuffer cmdB,
                    float planetRadius, const double3& planetCenter, float toggleDistance, float triangleSize, uint32_t materialID,
                    const CPUMesh& mesh, const CBT& cbt);
    void release();

    // Reload shaders
    void reload_shaders(const std::string& shaderLibrary);

    // Pre-render loop call
    void upload_static_constant_buffers(CommandBuffer cmd);

    // Update the constant buffers
    void update_constant_buffers(CommandBuffer cmd, const UpdateProperties& updateProperties);

    // Render planet
    void evaluate_leb(CommandBuffer cmdB, const Camera& camera, ConstantBuffer globalCB, GraphicsBuffer lebMatrixCache, bool clear, bool complete = false);

    // UI
    void render_ui(const char* planetName);

    // Is the panet close to the camera
    void planet_visibility(const Camera& camera, double& distance, bool& visible, bool& updatable) const;
    const CBTMesh& get_cbt_mesh() const { return m_CBTMesh; }
    CBTMesh& get_cbt_mesh() { return m_CBTMesh; }
    const BaseMesh& get_base_mesh() const { return m_BaseMesh; }
    BaseMesh& get_base_mesh() { return m_BaseMesh; }
    ConstantBuffer get_geometry_cb() const { return m_GeometryCB; }
    ConstantBuffer get_planet_cb() const { return m_PlanetCB; }
    ConstantBuffer get_update_cb() const { return m_UpdateCB; }

private:
    // General
    GraphicsDevice m_Device = 0;

    // Static properties
    float m_PlanetRadius = 0.0;
    float m_ToggleDistance = 0.0;
    double3 m_PlanetCenter = { 0.0, 0.0, 0.0 };
    uint32_t m_MaterialID = UINT32_MAX;

    // Modifiable properties
    int32_t m_MaxSubdivisionDepth = 0;
    float m_TriangleSize = 0.0;

    // Runtime resources
    CBTMesh m_CBTMesh = CBTMesh();
    BaseMesh m_BaseMesh = BaseMesh();
    ConstantBuffer m_GeometryCB = 0;
    ConstantBuffer m_PlanetCB = 0;
    ConstantBuffer m_UpdateCB = 0;

    // Compute shader
    ComputeShader m_LebEvalCS = 0;
    ComputeShader m_ClearCS = 0;
};
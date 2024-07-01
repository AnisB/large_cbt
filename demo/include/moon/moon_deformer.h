#pragma once

// Project includes
#include "graphics/descriptors.h"
#include "graphics/types.h"
#include "mesh/mesh.h"
#include "moon/moon_material.h"
#include "render_pipeline/camera.h"
#include "render_pipeline/planet.h"
#include "render_pipeline/sky.h"

class MoonDeformer
{
public:
    // Cst & Dst
    MoonDeformer();
    ~MoonDeformer();

    // Init and release
    void initialize(GraphicsDevice device, const char* textureLibrary);
    void release();

    // Shader reload
    void reload_shaders(const std::string& shaderLibrary);

    // Apply deformation to mesh
    void apply_deformation(CommandBuffer cmdB, const MoonMaterial& moonMat, ConstantBuffer globalCB, const CBTMesh& mesh, ConstantBuffer geometryCB, ConstantBuffer planetCB);

private:
    // Generic graphics
    GraphicsDevice m_Device = 0;
    std::string m_ShaderLibrary = "";

    // Rendering resource
    ComputeShader m_EvaluateDeformationCS = 0;
    SamplerDescriptor m_LinearWrap = SamplerDescriptor();
};
#pragma once

// Project includes
#include "graphics/types.h"
#include "graphics/descriptors.h"
#include "mesh/mesh.h"
#include "water/water_data.h"

class WaterDeformer
{
public:
    // Cst & Dst
    WaterDeformer();
    ~WaterDeformer();

    // Init and release
    void initialize(GraphicsDevice device);
    void release();

    // Shader reload
    void reload_shaders(const std::string& shaderLibrary);

    // Apply deformation to mesh
    void apply_deformation(CommandBuffer cmdB, const CBTMesh& mesh, const WaterData& waterData, ConstantBuffer globalCB, ConstantBuffer geometryCB, ConstantBuffer planetCB, ConstantBuffer updateCB);

private:
    // Generic graphics
    GraphicsDevice m_Device = 0;
    std::string m_ShaderLibrary = "";

    // Rendering resource
    ComputeShader m_EvaluateDeformationCS = 0;
    SamplerDescriptor m_LinearWrap = SamplerDescriptor();
};

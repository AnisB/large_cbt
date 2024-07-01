// Project includes
#include "tools/shader_utils.h"

// Project includes
#include "graphics/dx12_backend.h"
#include "tools/security.h"

void compile_and_replace_compute_shader(GraphicsDevice device, const ComputeShaderDescriptor& csd, ComputeShader& oldCS, bool experimental)
{
    // Compile the shader
    ComputeShader newCs = d3d12::compute_shader::create_compute_shader(device, csd, experimental);

    // If it succeded to compile replace.
    if (newCs != 0)
    {
        // Destroy the previously existing
        if (oldCS != 0)
            d3d12::compute_shader::destroy_compute_shader(oldCS);
        oldCS = newCs;
    }
    assert_msg(oldCS != 0, (csd.filename + " failed to compile.").c_str());
}

void compile_and_replace_graphics_pipeline(GraphicsDevice device, const GraphicsPipelineDescriptor& gpd, GraphicsPipeline& oldGP)
{
    // Compile the graphics pipeline
    GraphicsPipeline newGP = d3d12::graphics_pipeline::create_graphics_pipeline(device, gpd);

    // If it succeded to compile replace.
    if (newGP != 0)
    {
        // Destroy the previously existing
        if (oldGP != 0)
            d3d12::graphics_pipeline::destroy_graphics_pipeline(oldGP);
        oldGP = newGP;
    }
    assert_msg(oldGP != 0, (gpd.filename + " failed to compile.").c_str());
}
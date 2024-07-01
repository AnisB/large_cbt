#pragma once

// Project includes
#include "graphics/descriptors.h"

// Compile a shader and replace if succeded
void compile_and_replace_compute_shader(GraphicsDevice device, const ComputeShaderDescriptor& csd, ComputeShader& oldCS, bool experimental = false);

// Compile a graphics pipeline and replace if succeded
void compile_and_replace_graphics_pipeline(GraphicsDevice device, const GraphicsPipelineDescriptor& gpd, GraphicsPipeline& oldGP);
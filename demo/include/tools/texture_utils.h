#pragma once

// Project includes
#include "graphics/descriptors.h"

#if defined(SUPPORT_SPNG)
// Load a png to a graphics buffer
GraphicsBuffer load_image_to_graphics_buffer_png(GraphicsDevice device, const char* texturePath, uint32_t& width, uint32_t& height, TextureFormat& textureFormat);
#endif

#if defined(SUPPORT_TIFF)
// Load a tiff to a graphics buffer
GraphicsBuffer load_image_to_graphics_buffer_tiff(GraphicsDevice device, const char* texturePath, uint32_t& width, uint32_t& height, TextureFormat& textureFormat);
#endif


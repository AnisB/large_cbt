#pragma once

// Stencil flags for the rendering passes
enum class StencilUsage
{
	Background = 0x01,
	Foreground = 0x02,
	SkyHost = 0x04,
	SkyOccluder = 0x08
};
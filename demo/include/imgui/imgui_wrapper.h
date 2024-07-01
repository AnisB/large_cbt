#pragma once

// Project includes
#include "graphics/types.h"
#include "graphics/event_collector.h"

namespace imgui_d3d12
{
	// Init & Dst
	bool initialize_imgui(GraphicsDevice device, RenderWindow window);
	void release_imgui();

	// Runtime functions
	void start_frame();
	void end_frame();
	void draw_frame(CommandBuffer cmd, RenderTexture renderTexture);
	void handle_input(RenderWindow window, const EventData& data);
}

#pragma once

// System includes
#include <queue>

enum class MouseButton
{
	Left,
	Middle,
	Right,
	Other
};

enum class FrameEvent
{
	Close,
	Destroy,
	MouseMovement,
	MouseWheel,
	MouseButton,
	KeyDown,
	KeyUp,
	Raw
};

struct EventData
{
	FrameEvent type;
	uint32_t data0;
	uint64_t data1;
	int64_t data2;
};

namespace event_collector
{
	void push_event(const EventData& event);
	bool peek_event(EventData& event);
	void request_draw();
	bool active_draw_request();
	void draw_done();
}

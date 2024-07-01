// Internal includes
#include "graphics/event_collector.h"

namespace event_collector
{
	// Queue that is used to keep track of the events
	static std::queue<EventData> eventQueue;

	// Flag that tracks if a rendering should be done
	static bool drawRequested = false;

	// If an event has been recorded process it
	bool peek_event(EventData& event)
	{
		if (eventQueue.size() > 0)
		{
			event = eventQueue.front();
			eventQueue.pop();
			return true;
		}
		return false;
	}

	// Keep track of this event
	void push_event(const EventData& event)
	{
		eventQueue.push(event);
	}

	void request_draw()
	{
		drawRequested = true;
	}

	bool active_draw_request()
	{
		return drawRequested;
	}

	void draw_done()
	{
		drawRequested = false;
	}
}

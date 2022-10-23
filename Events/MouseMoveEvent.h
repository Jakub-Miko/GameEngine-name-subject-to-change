#pragma once
#include "Event.h"


class MouseMoveEvent : public Event {
	EVENT_ID(MouseMoveEvent);
public:
	MouseMoveEvent(double x, double y) : x(x), y(y)
	{

	}

public:
	double x, y;
};


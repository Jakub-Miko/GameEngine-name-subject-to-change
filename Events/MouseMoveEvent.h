#pragma once
#include "Event.h"


class MouseMoveEvent : public Event {
public:
	MouseMoveEvent(double x, double y) : x(x), y(y)
	{

	}

	virtual EventType GetType() override {
		return MouseMoveEvent::GetStaticType();
	}

	static EventType GetStaticType() {
		return EventType::MOUSE_MOVE;
	}

public:
	double x, y;
};


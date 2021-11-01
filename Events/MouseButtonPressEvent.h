#pragma once
#include "Event.h"
#include "KeyCodes.h"
#include <cstdint>

class MouseButtonPressEvent : public Event {
public:
	MouseButtonPressEvent(MouseButtonCode key_code, KeyPressType press_type, KeyModifiers key_mods)
		: key_code(key_code), key_mods(key_mods), press_type(press_type)
	{

	}

	virtual EventType GetType() override {
		return MouseButtonPressEvent::GetStaticType();
	}

	static EventType GetStaticType() {
		return EventType::MOUSE_BUTTON_PRESS;
	}

public:
	MouseButtonCode key_code;
	KeyPressType press_type;
	KeyModifiers key_mods;
};


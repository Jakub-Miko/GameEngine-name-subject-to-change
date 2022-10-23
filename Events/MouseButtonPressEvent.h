#pragma once
#include "Event.h"
#include "KeyCodes.h"
#include <cstdint>

class MouseButtonPressEvent : public Event {
	EVENT_ID(MouseButtonPressEvent);
public:
	MouseButtonPressEvent(MouseButtonCode key_code, KeyPressType press_type, KeyModifiers key_mods)
		: key_code(key_code), key_mods(key_mods), press_type(press_type)
	{

	}

public:
	MouseButtonCode key_code;
	KeyPressType press_type;
	KeyModifiers key_mods;
};


#pragma once
#include "Event.h"
#include "KeyCodes.h"
#include <cstdint>
#include <Core/RuntimeTag.h>

class KeyPressedEvent : public Event {
	EVENT_ID(KeyPressedEvent);
public:
	KeyPressedEvent(KeyCode key_code, KeyPressType press_type, KeyModifiers key_mods) 
		: key_code(key_code), key_mods(key_mods), press_type(press_type)
	{

	}

public:
	KeyCode key_code;
	KeyPressType press_type;
	KeyModifiers key_mods;
};


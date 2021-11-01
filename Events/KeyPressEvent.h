#pragma once
#include "Event.h"
#include "KeyCodes.h"
#include <cstdint>

class KeyPressedEvent : public Event {
public:
	KeyPressedEvent(KeyCode key_code, KeyPressType press_type, KeyModifiers key_mods) 
		: key_code(key_code), key_mods(key_mods), press_type(press_type)
	{

	}
	
	virtual EventType GetType() override {
		return KeyPressedEvent::GetStaticType();
	}

	static EventType GetStaticType() {
		return EventType::KEY_PRESS;
	}

public:
	KeyCode key_code;
	KeyPressType press_type;
	KeyModifiers key_mods;
};


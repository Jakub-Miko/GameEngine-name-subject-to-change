#pragma once

enum class EventType : unsigned char
{
	INVALID = 0, KEY_PRESS = 1, MOUSE_BUTTON_PRESS = 2 ,MOUSE_MOVE = 3, SCROLL = 4, EXIT = 5 
};


class Event {
public:
	virtual EventType GetType() = 0;

public:
	bool handled = false;
};



class EventDispacher {
public:
	EventDispacher(Event* e) : ev(e) {}

	template<typename Event_type,typename F>
	void Dispatch(F func) {
		if (Event_type::GetStaticType() == ev->GetType()) {
			ev->handled = func(reinterpret_cast<Event_type*>(ev));
		}
	}

private:
	Event* ev;
};
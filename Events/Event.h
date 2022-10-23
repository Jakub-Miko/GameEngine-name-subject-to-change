#pragma once
#include <Core/RuntimeTag.h>
#ifdef __INTELLISENSE__
//Passing template arguments through macros can cause intellisense to break, this is here to show intellisense a different version the function to stop that from happening.
#define EVENT_ID(EventType) RUNTIME_TAG(#EventType); virtual RuntimeTagIdType GetType() override { return -1; } 
#else
#define EVENT_ID(EventType) RUNTIME_TAG(#EventType); virtual RuntimeTagIdType GetType() override { return RuntimeTag<EventType>::GetId(); } 
#endif

enum class EventType : unsigned char
{
	INVALID = 0, KEY_PRESS = 1, MOUSE_BUTTON_PRESS = 2 ,MOUSE_MOVE = 3, SCROLL = 4, EXIT = 5 
};


class Event {
public:
	virtual RuntimeTagIdType GetType() = 0;

public:
	bool handled = false;
};



class EventDispacher {
public:
	EventDispacher(Event* e) : ev(e) {}

	template<typename Event_type,typename F>
	void Dispatch(F func) {
		if (RuntimeTag<Event_type>::GetId() == ev->GetType()) {
			ev->handled = func(reinterpret_cast<Event_type*>(ev));
		}
	}

private:
	Event* ev;
};
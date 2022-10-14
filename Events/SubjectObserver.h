#pragma once
#include <stdexcept>
#include <vector>
#include <Events/Event.h>

class EventSubject;

class EventObserver {
public:

	virtual void OnEvent(Event* e) = 0;

	void Reset();

	virtual ~EventObserver();

private:
	void Terminate();
	friend class EventSubject;
	EventSubject* subject = nullptr;
	int index;
};

class EventSubject {
public:

	EventSubject() : observers() {}

	void Notify(Event* e) {
		for (auto observer : observers) {
			observer->OnEvent(e);
		}
	}

	void Subscribe(EventObserver* observer) {
		if (observer->subject) throw std::runtime_error("This observer is already observing a different subject");
		observers.push_back(observer);
		observer->subject = this;
		observer->index = observers.size() - 1;

	}

	void Unsubscribe(EventObserver* observer) {
		if (observer->subject == this) {
			observer->Terminate();
			observers.back()->index = observer->index;
			std::swap(observers[observer->index], observers.back());
			observers.pop_back();
		}
		else {
			throw std::runtime_error("Can't unsubscribe an observer which was subscribed to a different subject");
		}
	}

	~EventSubject() {
		for (auto observer : observers) {
			observer->Terminate();
		}
	}

private:
	std::vector<EventObserver*> observers;

};

inline void EventObserver::Terminate()
{
	subject = nullptr;
}

inline void EventObserver::Reset()
{
	if (subject) {
		subject->Unsubscribe(this);
	}
}

inline EventObserver::~EventObserver()
{
	if (subject) {
		subject->Unsubscribe(this);
	}
}
#pragma once
#include <stdexcept>
#include <vector>
#include <Events/Event.h>
#include <mutex>

class EventSubject;

class EventObserverBase {
public:

	virtual void OnEvent(Event* e) = 0;

	void Reset();

	virtual ~EventObserverBase();

private:
	void Terminate();
	friend class EventSubject;
	EventSubject* subject = nullptr;
	int index;
};

template<typename Event_Type,typename func_type>
class EventObserver : public EventObserverBase {
public:
	EventObserver(func_type function) : function(function) {
		
	}

	virtual ~EventObserver() {

	}

	virtual void OnEvent(Event* e) override {
		EventDispacher dispatcher(e);
		dispatcher.Dispatch<Event_Type>(function);
	}

private:
	func_type function;
};

template<typename Event_Type, typename func_type>
EventObserverBase* MakeEventObserver(func_type function) {
	return (EventObserverBase*)new EventObserver<Event_Type, func_type>(function);
}

class EventSubject {
public:

	EventSubject() : observers(), sync() {}

	void Notify(Event* e) {
		std::lock_guard<std::mutex> lock(sync);
		for (auto observer : observers) {
			observer->OnEvent(e);
		}
	}

	void Subscribe(EventObserverBase* observer) {
		std::lock_guard<std::mutex> lock(sync);
		if (observer->subject) throw std::runtime_error("This observer is already observing a different subject");
		observers.push_back(observer);
		observer->subject = this;
		observer->index = observers.size() - 1;

	}

	void Unsubscribe(EventObserverBase* observer) {
		std::lock_guard<std::mutex> lock(sync);
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
		std::lock_guard<std::mutex> lock(sync);
		for (auto observer : observers) {
			observer->Terminate();
		}
	}

private:
	std::mutex sync;
	std::vector<EventObserverBase*> observers;

};

inline void EventObserverBase::Terminate()
{
	subject = nullptr;
}

inline void EventObserverBase::Reset()
{
	if (subject) {
		subject->Unsubscribe(this);
	}
}

inline EventObserverBase::~EventObserverBase()
{
	if (subject) {
		subject->Unsubscribe(this);
	}
}
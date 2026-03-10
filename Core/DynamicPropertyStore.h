#pragma once
#include <memory>
#include <string>
#include <unordered_map>

#include "DynamicProperties.h"
#include "Events/SubjectObserver.h"

class DynamicPropertyStoreUpdateEvent : public Event {
public:
    EVENT_ID(DynamicPropertyStoreUpdateEvent)
    DynamicPropertyStoreUpdateEvent() = default;

    std::shared_ptr<DynamicPropertyBase> updated_property;
};

class DynamicPropertyStore {
public:
    DynamicPropertyStore() = default;

    template<typename T>
    std::shared_ptr<DynamicProperty<T>> GetProperty(const std::string& name) {
        auto fnd = properties.find(name);
        if (fnd != properties.end()) {
            return std::static_pointer_cast<DynamicProperty<T>>(fnd->second);
        } else {
            return nullptr;
        }
    }

    template<typename T>
    std::pair<bool, std::shared_ptr<DynamicProperty<T>>> SetProperty(const std::string& name, const T& value) {
        auto fnd = properties.find(name);
        if (fnd != properties.end()) {
            if(!fnd->second->SetValue(value)) {
                throw std::runtime_error("Could not set property " + name + " to value, ensure the type matches.");
            }
            auto update_event = std::make_unique<DynamicPropertyStoreUpdateEvent>();
            update_event->updated_property = fnd->second;
            update_event_subject.Notify(update_event.get());
            return std::make_pair(true, std::dynamic_pointer_cast<DynamicProperty<T>>(fnd->second));
        } else {
            auto new_prop = std::make_shared<DynamicProperty<T>>(name, value);
            properties.insert(std::make_pair(name, new_prop));
            auto update_event = std::make_unique<DynamicPropertyStoreUpdateEvent>();
            update_event->updated_property = new_prop;
            update_event_subject.Notify(update_event.get());
            return std::make_pair(false, new_prop);
        }
    }

    void ClearProperty(const std::string& name) {
        properties.erase(name);
    }

    std::vector<std::shared_ptr<DynamicPropertyBase>> GetProperties() const {
        std::vector<std::shared_ptr<DynamicPropertyBase>> props;
        for(auto& prop : properties) {
            props.push_back(prop.second);
        }
        return std::move(props);
    }

    void RegisterPropertyUpdateObserver(EventObserverBase* observer) {
        update_event_subject.Subscribe(observer);
    }

private:
    EventSubject update_event_subject;
    std::unordered_map<std::string, std::shared_ptr<DynamicPropertyBase>> properties;
};

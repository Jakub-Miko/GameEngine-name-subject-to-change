#pragma once
#include <memory>
#include <string>
#include <unordered_map>

#include "DynamicProperties.h"
#include "Events/SubjectObserver.h"

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
            return std::make_pair(true, std::dynamic_pointer_cast<DynamicProperty<T>>(fnd->second));
        } else {
            auto new_prop = std::make_shared<DynamicProperty<T>>(name, value);
            properties.insert(std::make_pair(name, new_prop));
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
        return props;
    }

private:
    std::unordered_map<std::string, std::shared_ptr<DynamicPropertyBase>> properties;
};

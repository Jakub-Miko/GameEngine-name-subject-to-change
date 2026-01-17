#pragma once
#include <any>
#include <string>

#include "DynamicPropertyCustomTypes.h"
#include "TypeId.h"
#include "TypeList.h"


class DynamicPropertyBase {
public:

    using Types = TypeList<std::string, MultiChoice, bool>;

    virtual std::string GetName() = 0;
    virtual std::string GetTypeName() = 0;
    virtual std::any GetValue() = 0;
    virtual bool SetValue(const std::any& value) = 0;

    virtual ~DynamicPropertyBase() {};
};

template<typename T>
class DynamicProperty : public DynamicPropertyBase, public std::enable_shared_from_this<DynamicProperty<T>> {
public:
    DynamicProperty() = default;
    DynamicProperty(std::string name, const T& value) : value(value), name(name) {}
    DynamicProperty(std::string name, T&& value) : value(std::move(value)), name(name) {}

    std::string GetName() override {
        return name;
    }

    std::string GetTypeName() {
        return std::string(RuntimeTag<T>::GetName());
    }

    std::any GetValue() {
        return value;
    };

    T& GetValueTyped() {
        return value;
    };

    bool SetValue(const std::any& new_value) override {
        if(new_value.type() != typeid(T)) {
            return false;
        }
        this->value = std::any_cast<const T&>(new_value);
        return true;
    }

    void SetValueTyped(T& new_value) {
        this->value = new_value;
    }
    ~DynamicProperty() override = default;

private:
    std::string name;
    T value;
};

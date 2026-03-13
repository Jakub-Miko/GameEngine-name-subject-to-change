#pragma once


#define MAX_INPUT_TEXT_SIZE 500
#include <cstdint>
#include <cstring>
#include <memory>
#include <string>

#include "Core/DynamicProperties.h"
#include "Core/DynamicPropertyModuleAdapterBase.h"


template<typename T>
class UIModuleAdapter;

class UIModuleAdapterBase : public PropertyModuleBase {
public:
    template<typename T>
    using Implementation = UIModuleAdapter<T>;

    virtual void RenderUI() { };

    bool IsImplemented() override { return true;}
    ~UIModuleAdapterBase() override {}
};

template<typename T>
class UIModuleAdapter : public UIModuleAdapterBase {
public:
    UIModuleAdapter() {}
    UIModuleAdapter(std::shared_ptr<DynamicProperty<T>> value) {}
    ~UIModuleAdapter() override {}
    bool IsImplemented() override { return false; }
    void RenderUI() override {}
};

template<>
class UIModuleAdapter<std::string> : public UIModuleAdapterBase {
public:
    UIModuleAdapter() {}
    UIModuleAdapter(std::shared_ptr<DynamicProperty<std::string>> value) : value(value), text_buffer(new char[MAX_INPUT_TEXT_SIZE]) {
        std::memcpy(text_buffer.get(), value->GetValueTyped().c_str(), std::min((size_t)MAX_INPUT_TEXT_SIZE, value->GetValueTyped().size()));
    }

    void RenderUI() override;

    ~UIModuleAdapter() override {}
private:
    std::unique_ptr<char[]> text_buffer;
    std::shared_ptr<DynamicProperty<std::string>> value;
};

template<>
class UIModuleAdapter<MultiChoice> : public UIModuleAdapterBase {
public:
    UIModuleAdapter() {}
    UIModuleAdapter(std::shared_ptr<DynamicProperty<MultiChoice>> value) : value(value), text_buffer(new char[MAX_INPUT_TEXT_SIZE]) {
        std::memcpy(text_buffer.get(), value->GetValueTyped().GetValue().c_str(), std::min((size_t)MAX_INPUT_TEXT_SIZE, value->GetValueTyped().GetValue().size()));
    }

    void RenderUI() override;

    ~UIModuleAdapter() override {}
private:
    std::unique_ptr<char[]> text_buffer;
    std::shared_ptr<DynamicProperty<MultiChoice>> value;
};

template<>
class UIModuleAdapter<bool> : public UIModuleAdapterBase {
public:
    UIModuleAdapter() {}
    UIModuleAdapter(std::shared_ptr<DynamicProperty<bool>> value) : value(value) {}

    void RenderUI() override;

    ~UIModuleAdapter() override {}
private:
    std::shared_ptr<DynamicProperty<bool>> value;
};

template<>
class UIModuleAdapter<float> : public UIModuleAdapterBase {
public:
    UIModuleAdapter() {}
    UIModuleAdapter(std::shared_ptr<DynamicProperty<float>> value) : value(value) {}

    void RenderUI() override;

    ~UIModuleAdapter() override {}
private:
    std::shared_ptr<DynamicProperty<float>> value;
};

template<>
class UIModuleAdapter<uint32_t> : public UIModuleAdapterBase {
public:
    UIModuleAdapter() {}
    UIModuleAdapter(std::shared_ptr<DynamicProperty<uint32_t>> value) : value(value) {}

    void RenderUI() override;

    ~UIModuleAdapter() override {}
private:
    std::shared_ptr<DynamicProperty<uint32_t>> value;
};

template<>
class UIModuleAdapter<DynamicPropertyAction> : public UIModuleAdapterBase {
public:
    UIModuleAdapter() {}
    UIModuleAdapter(std::shared_ptr<DynamicProperty<DynamicPropertyAction>> value) : value(value) {}

    void RenderUI() override;

    ~UIModuleAdapter() override {}
private:
    std::shared_ptr<DynamicProperty<DynamicPropertyAction>> value;
};
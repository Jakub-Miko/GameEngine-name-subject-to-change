#pragma once
class PropertyModuleBase {
public:
    virtual bool IsImplemented() = 0;
    virtual ~PropertyModuleBase() = default;
};
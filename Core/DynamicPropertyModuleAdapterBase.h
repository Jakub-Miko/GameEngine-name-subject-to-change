#pragma once

/**
 * @brief A common BaseClass for polymorphic template classes used by DynamicPropertyProjector
 */
class PropertyModuleBase {
public:
    virtual bool IsImplemented() = 0;
    virtual ~PropertyModuleBase() = default;
};
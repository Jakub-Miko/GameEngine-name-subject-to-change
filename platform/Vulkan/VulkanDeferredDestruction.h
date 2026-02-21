#pragma once

/**
 * @brief Abstract class used to defer resource destruction until the resource is no longer used on the GPU. 
 */
class VulkanDeferredDestruction {
public:
    /**
     * @brief Destroy the resource after it is no longer used 
     * 
     * This is called internally bu the RenderResource manager which calls it when the last resource usage timeline value has passed.
     * 
     * It is possible the resource might be reused upon destruction so the return value dictates whether it should or should not be destroyed.
     * 
     * @return true The resource can be deleted.
     * @return false The resource cannot be deleted
     */
    virtual bool Destroy() = 0;
    virtual ~VulkanDeferredDestruction() = default;
};
#pragma once
#include "RendererDefines.h"
#include "RenderResource.h"
#include <Events/Event.h>
#include <Events/SubjectObserver.h>

/**
 * @brief Base type used to pass implementation specific data about a present event
 * 
 * Used when the engine wants to present to all registered surfaces.
 */
class RenderPresentEvent : public Event {
public:
    EVENT_ID(RenderPresentEvent);
};

/**
 * @brief Abstraction over presentable surface, facilitate output to windows and swapchain framebuffers to render to.
 */
class RenderSurface {
public:
    virtual std::shared_ptr<RenderFrameBufferResource> GetFrameBufferByIndex(int index) = 0;
    virtual std::shared_ptr<RenderFrameBufferResource> GetCurrentFrameBuffer() = 0;
    virtual int GetCurrentFramebufferIndex() = 0;
};

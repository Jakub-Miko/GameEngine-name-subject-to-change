#include "RenderCommandList.h"
#include "platform/OpenGL/OpenGLRenderCommandList.h"
#include <Renderer/Renderer.h>


RenderCommandList::RenderCommandList(Renderer* renderer, RenderCommandAllocator* alloc)
    :m_Renderer(renderer), m_Alloc(alloc)
{
}

RenderCommandList* RenderCommandList::CreateQueue(Renderer* renderer, RenderCommandAllocator* alloc){
    
    return new OpenGLRenderCommandList(renderer, alloc);

}

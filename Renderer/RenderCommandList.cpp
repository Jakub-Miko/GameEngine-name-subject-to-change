#include "RenderCommandList.h"
#include "platform/OpenGL/OpenGLRenderCommandList.h"
#include <Renderer/Renderer.h>


RenderCommandList::RenderCommandList(Renderer* renderer)
    :m_Renderer(renderer)
{
}

RenderCommandList* RenderCommandList::CreateQueue(Renderer* renderer){
    
    return new OpenGLRenderCommandList(renderer);

}

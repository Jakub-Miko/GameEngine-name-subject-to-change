#include "OpenGLRenderCommandList.h"
#include "OpenGLDrawCommand.h"
#include "OpenGLBindOpenGLContextCommand.h"
#include "OpenGLRenderCommand.h"
#include "OpenGLRenderCommandAllocator.h"
#include "OpenGLSwapBuffersCommand.h"
#include <memory>

#include <memory_resource>
#include <type_traits>
#include <Renderer/Renderer.h>
#include <Profiler.h>
#include <utility>


template<typename T, typename ...Args>
void OpenGLRenderCommandList::PushCommand(Args&& ...args)
{
    OpenGLRenderCommandAllocator::Pool* pool = reinterpret_cast<OpenGLRenderCommandAllocator::Pool*>(m_Alloc->Get());
    std::pmr::polymorphic_allocator<T> alloc(pool);

    T* cmd = std::allocator_traits<decltype(alloc)>::allocate(alloc, 1);
    std::allocator_traits<decltype(alloc)>::construct(alloc, cmd, std::forward<Args>(args)...);
    if (!m_Commands) {
        m_Commands = cmd;
        m_Commands_tail = cmd;
    }
    else {
        m_Commands_tail->next = cmd;
        m_Commands_tail = cmd;
    }
}


OpenGLRenderCommandList::OpenGLRenderCommandList(Renderer* renderer, std::shared_ptr<RenderCommandAllocator> alloc)
    : RenderCommandList(renderer, alloc)
{

}

void OpenGLRenderCommandList::DrawSquare(glm::vec2 pos, glm::vec2 size, glm::vec4 color) {
    PROFILE("Draw Square Command");
    PushCommand<OpenGLDrawCommand>(pos, size, color);
}

void OpenGLRenderCommandList::SwapBuffers()
{
    PushCommand<OpenGLSwapBuffersCommand>();
}

void OpenGLRenderCommandList::BindOpenGLContext()
{
    PushCommand<OpenGLBindOpenGLContextCommand>();
}

void OpenGLRenderCommandList::Execute()
{
    OpenGLRenderCommand* next = m_Commands;
    while(next) {
        PROFILE("Command Execution");
        next->Execute();
        next = next->next;
    }
    m_Commands = nullptr;
}

OpenGLRenderCommandList::~OpenGLRenderCommandList()
{
    m_Commands = nullptr;
}

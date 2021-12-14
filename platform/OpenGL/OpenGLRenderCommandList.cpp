#include "OpenGLRenderCommandList.h"
#include "OpenGLDrawCommand.h"
#include "OpenGLBindOpenGLContextCommand.h"
#include "OpenGLRenderCommand.h"
#include "OpenGLRenderCommandAllocator.h"
#include <memory>

#include <memory_resource>
#include <type_traits>
#include <Renderer/Renderer.h>
#include <Profiler.h>
#include <utility>


template<typename T, typename ...Args>
void OpenGLRenderCommandList::PushCommand(Args&& ...args)
{
    std::allocator<T> alloc;

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


OpenGLRenderCommandList::OpenGLRenderCommandList(Renderer* renderer)
    : RenderCommandList(renderer)
{

}

void OpenGLRenderCommandList::DrawSquare(glm::vec2 pos, glm::vec2 size, glm::vec4 color) {
    PushCommand<OpenGLDrawCommand>(pos, size, color);
}

void OpenGLRenderCommandList::DrawSquare(const glm::mat4& transform, glm::vec4 color)
{
    PushCommand<OpenGLDrawCommand>(transform, color);
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

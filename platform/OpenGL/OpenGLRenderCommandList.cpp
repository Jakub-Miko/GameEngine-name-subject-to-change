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

OpenGLRenderCommandList::OpenGLRenderCommandList(Renderer* renderer, std::shared_ptr<RenderCommandAllocator> alloc)
    : RenderCommandList(renderer, alloc)
{

}

void OpenGLRenderCommandList::DrawSquare(glm::vec2 pos, glm::vec2 size, glm::vec4 color) {
    PROFILE("Draw Square Command");
    OpenGLRenderCommandAllocator::Pool* pool = reinterpret_cast<OpenGLRenderCommandAllocator::Pool*>(m_Alloc->Get());
    std::pmr::polymorphic_allocator<OpenGLDrawCommand> alloc(pool);

    OpenGLDrawCommand* cmd = std::allocator_traits<decltype(alloc)>::allocate(alloc, 1);
    std::allocator_traits<decltype(alloc)>::construct(alloc, cmd, pos, size, color);
    cmd->next = m_Commands;
    m_Commands = cmd;
}

void OpenGLRenderCommandList::BindOpenGLContext()
{
    OpenGLRenderCommandAllocator::Pool* pool = reinterpret_cast<OpenGLRenderCommandAllocator::Pool*>(m_Alloc->Get());
    std::pmr::polymorphic_allocator<OpenGLBindOpenGLContextCommand> alloc(pool);

    OpenGLBindOpenGLContextCommand* cmd = std::allocator_traits<decltype(alloc)>::allocate(alloc, 1);
    std::allocator_traits<decltype(alloc)>::construct(alloc, cmd);
    cmd->next = m_Commands;
    m_Commands = cmd;
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

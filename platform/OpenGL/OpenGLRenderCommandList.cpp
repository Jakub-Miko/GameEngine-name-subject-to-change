#include "OpenGLRenderCommandList.h"
#include "OpenGLDrawCommand.h"
#include "OpenGLRenderCommand.h"
#include "OpenGLRenderCommandAllocator.h"
#include <memory>
#include <memory_resource>
#include <type_traits>
#include <Renderer/Renderer.h>
#include <Profiler.h>

OpenGLRenderCommandList::OpenGLRenderCommandList(Renderer* renderer, RenderCommandAllocator* alloc)
    : RenderCommandList(renderer, alloc)
{

}

void OpenGLRenderCommandList::DrawSquare(glm::vec2 pos, glm::vec2 size, glm::vec4 color) {
    PROFILE("Draw Square Command");
    OpenGLRenderCommandAllocator::Pool* pool = reinterpret_cast<OpenGLRenderCommandAllocator::Pool*>(m_Alloc->Get());
    std::pmr::polymorphic_allocator<OpenGLDrawCommand> alloc(pool);

    OpenGLDrawCommand* cmd = std::allocator_traits<decltype(alloc)>::allocate(alloc, 1);
    std::allocator_traits<decltype(alloc)>::construct(alloc, cmd, pos, size, color);

    m_Commands.push_back(cmd);
}

void OpenGLRenderCommandList::Submit() {
    m_Renderer->SubmitQueue(this);
}

void OpenGLRenderCommandList::Execute()
{
    for (auto command: m_Commands) {
        PROFILE("Command Execution");
        command->Execute();
    }
    m_Commands.clear();
}

OpenGLRenderCommandList::~OpenGLRenderCommandList()
{
    m_Commands.clear();
}

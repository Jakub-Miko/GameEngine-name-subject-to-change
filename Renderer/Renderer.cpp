#include "Renderer.h"
#include <ThreadManager.h>
#include <Renderer/TextRenderer.h>
#include <Application.h>
#include <World/Components/CameraComponent.h>
#include <Renderer/Renderer3D/Renderer3D.h>
#include <platform/OpenGL/OpenGLRenderCommandList.h>
#include <Renderer/RenderContext.h>
#include <Renderer/RenderResourceManager.h>
#include <Renderer/ShaderManager.h>
#include <Renderer/PipelineManager.h>
#include <stb_image.h>
#include <Renderer/TextureManager.h>

Renderer* Renderer::instance = nullptr;

Renderer* Renderer::Get() {
    return instance;
}

Renderer::Renderer() {
    RenderContext::Create();
} 

void Renderer::PreInit() {
    //stbi_set_flip_vertically_on_load(true); // This gets in vulkans way
    if(RenderContext::Get()) {
        RenderContext::Get()->PreInit();
    }
    RenderResourceManager::Initialize();
}

 std::shared_ptr<RenderCommandList> Renderer::GetRenderCommandList()
{
    auto manager = ThreadManager::Get();
    if(!manager->IsValidThreadContext()) {
        throw std::runtime_error("GetRenderCommandList can only be called in a thread managed by ThreadManager, you can still allocate command lists through allocators directly.\n");
    }
    
    if(manager->ThreadLocalDataExists<PerThreadRendererData>()) {
        auto data = manager->GetThreadLocalData<PerThreadRendererData>();
        return data->general_allocator->GetCommandList();
    } else {
        auto data = std::make_shared<PerThreadRendererData>();
        data->general_allocator = RenderCommandAllocator::CreateAllocator(1024);
        manager->SetThreadLocalData(data);
        return data->general_allocator->GetCommandList();
    }
}

void Renderer::Init() {
    ShaderManager::Initialize();
    PipelineManager::Initialize();
    if (RenderContext::Get()) {
        RenderContext::Get()->Init();
    }
    TextureManager::Init();
}

void Renderer::PostInit()
{
    Renderer3D::Init();
    TextRenderer::Init();
}


RenderCommandQueue* Renderer::GetCommandQueue(RenderQueueTypes type)
{
    return m_CommandQueues[(unsigned char)type];
}

RenderFence* Renderer::GetFence()
{
    return RenderFence::CreateFence();
}

void Renderer::Shutdown()
{
    TextRenderer::Shutdown();
    Renderer3D::PreShutdown();
    std::unique_lock<std::mutex> lock(instance->default_frame_buffer_mutex);
    if (instance->default_frame_buffer) {
        instance->default_frame_buffer.reset();
    }
    lock.unlock();
    ShaderManager::Shutdown();
    Renderer3D::Shutdown();
    RenderContext::Get()->StartShutdown();
    PipelineManager::Shutdown();
    RenderResourceManager::Shutdown();
    if (instance) {
        instance->Destroy();
        delete instance;
    }
}

void Renderer::SetRenderQueue(RenderCommandQueue* queue, RenderQueueTypes type)
{
    m_CommandQueues[(unsigned char)type] = queue;
}

void Renderer::Destroy()
{
    //Destroy all per thread command allocators
    for(auto thread : ThreadManager::Get()->GetAllThreadObjects()) {
        if(thread->StateValueExists<PerThreadRendererData>()) {
            auto per_thread_data = thread->GetStateValue<PerThreadRendererData>();
            per_thread_data->general_allocator.reset();
        }
    }
    RenderContext::Shutdown();
}

void Renderer::Create()
{
    if (!instance) {
        instance = new Renderer();
    }
}

void Renderer::Update(float delta_time)
{
    RenderResourceManager::Get()->Update();
    auto primary = Application::GetWorld().GetPrimaryEntity();
    if (primary == Entity() || !Application::GetWorld().EntityExists(primary) || !Application::GetWorld().HasComponent<CameraComponent>(primary)) {
        Application::GetWorld().SetPrimaryEntity(Entity());
        Application::GetWorld().CheckCamera();
        primary = Application::GetWorld().GetPrimaryEntity();
    } 
        
    glm::mat4 view_matrix = Application::GetWorld().GetComponent<TransformComponent>(primary).TransformMatrix;
    view_matrix[0] /= glm::length(view_matrix[0]);
    view_matrix[1] /= glm::length(view_matrix[1]);
    view_matrix[2] /= glm::length(view_matrix[2]);
    view_matrix[0] = glm::vec4(glm::normalize(glm::cross((glm::vec3)view_matrix[1], (glm::vec3)view_matrix[2])),0.0f);
    view_matrix[1] = glm::vec4(glm::normalize(glm::cross((glm::vec3)view_matrix[2], (glm::vec3)view_matrix[0])), 0.0f);
    glm::vec3 lengths = glm::vec3{ glm::length(view_matrix[0]) , glm::length(view_matrix[1]) , glm::length(view_matrix[2]) };

    Application::GetWorld().GetComponent<TransformComponent>(primary).TransformMatrix = view_matrix;
    
    Renderer3D::Get()->Update(delta_time);
    TextRenderer::Get()->UpdateLoadedFonts();
    TextRenderer::Get()->TextRenderSystem();
}
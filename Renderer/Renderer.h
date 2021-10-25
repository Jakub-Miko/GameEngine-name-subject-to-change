#pragma once
#include <Renderer/RenderCommandList.h>
#include <Renderer/RenderCommandAllocator.h>
#include <memory>
#include <vector>
#include <mutex>

class Renderer {
public: 

    Renderer(const Renderer& ref) = delete;
    Renderer(Renderer&& ref) = delete;
    Renderer& operator=(const Renderer& ref) = delete;
    Renderer& operator=(Renderer&& ref) = delete;
    
    void PreInit();

    RenderCommandList* GetRenderCommandList();

    RenderCommandAllocator* GetCommandAllocator();

    void Init();

    void SubmitQueue(RenderCommandList* queue);

    void Render();

    void Destroy();

private:
    Renderer() = default;
    
    std::vector<RenderCommandList*> m_Lists;
    std::vector<RenderCommandAllocator*> m_Allocators;
    std::vector<RenderCommandAllocator*> m_FreeAllocators;
    std::mutex m_List_mutex;
    static Renderer* instance;
public:
    
    static Renderer* Get();
};

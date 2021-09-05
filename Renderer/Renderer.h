#pragma once
#include <Renderer/RenderQueue.h>
#include <memory>
#include <vector>
#include <mutex>

class Renderer : public std::enable_shared_from_this<Renderer> {
public: 

    void PreInit();

    RenderQueue* GetRenderQueue();

    void Init();

    void SubmitQueue(RenderQueue* queue);

    void Render();

    ~Renderer();

private:
    std::vector<RenderQueue*> m_Queues;
    std::mutex m_Queue_mutex;
public:
    
    static Renderer* CreateRenderer();
};

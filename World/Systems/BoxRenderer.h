#pragma once
#include <Renderer/Renderer.h>
#include <World/Components/BoundingVolumeComponent.h>
#include <World/Components/TransformComponent.h>
#include <World/Components/CameraComponent.h>
#include <Renderer/PipelineManager.h>
#include <Renderer/RenderResourceManager.h>
#include <Renderer/ShaderManager.h>
#include <glm/glm.hpp>
#include <World/System.h>
#include <Core/FrameMultiBufferResource.h>

/**
 * @brief template used with std::visit, it contains a call operator with overloads matching function objects Ts
 * @tparam ...Ts fucntion objects used to provide overload for every type within a variant, so Overload can be used with std::visit
*/
template<typename ... Ts>                             
struct Overload : Ts ... { 
    using Ts::operator() ...;
};
template<class... Ts> Overload(Ts...) -> Overload<Ts...>;

/**
 * @brief Internal state used by the Render_Box system to draw boxes in a scene.
 * 
 * @deprecated Only used for drawing debugging visualization elements, otherwise this is just an artifact from the early stages of the Engines development, before a proper
 * rendering pipeline was implemented
*/
struct Render_Box_data {
    
    /**
     * @brief a Constant buffer containing constant data passed to the shader to render a shaded box
    */
    struct Constant_buffer_type {
        glm::mat4 mvp_matrix; ///< the MVP matrix contaning the necessary transformations for the boxes vertecies
        glm::mat4 model; ///< the model matrix contaning the boxes world transformation
        glm::vec4 sun_direction; ///< the direction of the sun (no longer used since the actual rendering pipeline has multiple lightsources)
        glm::vec4 color; ///< the color of the box
        glm::vec4 options; ///< extra vector containing additional settings
    };

    /**
     * @brief default constructor 
    */
    Render_Box_data() : pipeline(),
    vertex_buffer(),
    index_buffer(),
    constant_buffer() {}

    /**
     * @brief Frees the resources used by the internal state
    */
    void clear() {
        vertex_buffer.reset();
        index_buffer.reset();
        constant_buffer.reset();
    }

    /**
     * @brief Initliazes the state by allocating and generating the neccesary resources used by the rendering process
     * @param camera the camera component contaning the necessary projection information used for creating the projection matrix
     * @param camera_transform the camera transform used for creating the view matrix
    */
    Render_Box_data(const CameraComponent& camera, const glm::mat4& camera_transform) {
        float vertices[] = {
        0.5f, -0.5f, -0.5f, 1.0f,   0.0f,  0.0f, -1.0f, 1.0f,
       -0.5f, -0.5f, -0.5f, 1.0f,   0.0f,  0.0f, -1.0f, 1.0f,
        0.5f,  0.5f, -0.5f, 1.0f,   0.0f,  0.0f, -1.0f, 1.0f,
        0.5f,  0.5f, -0.5f, 1.0f,   0.0f,  0.0f, -1.0f, 1.0f,
       -0.5f,  0.5f, -0.5f, 1.0f,   0.0f,  0.0f, -1.0f, 1.0f,
       -0.5f, -0.5f, -0.5f, 1.0f,   0.0f,  0.0f, -1.0f, 1.0f,
       -0.5f, -0.5f,  0.5f, 1.0f,   0.0f,  0.0f,  1.0f, 1.0f,
        0.5f, -0.5f,  0.5f, 1.0f,   0.0f,  0.0f,  1.0f, 1.0f,
        0.5f,  0.5f,  0.5f, 1.0f,   0.0f,  0.0f,  1.0f, 1.0f,
        0.5f,  0.5f,  0.5f, 1.0f,   0.0f,  0.0f,  1.0f, 1.0f,
       -0.5f,  0.5f,  0.5f, 1.0f,   0.0f,  0.0f,  1.0f, 1.0f,
       -0.5f, -0.5f,  0.5f, 1.0f,   0.0f,  0.0f,  1.0f, 1.0f,
       -0.5f,  0.5f,  0.5f, 1.0f,  -1.0f,  0.0f,  0.0f, 1.0f,
       -0.5f,  0.5f, -0.5f, 1.0f,  -1.0f,  0.0f,  0.0f, 1.0f,
       -0.5f, -0.5f, -0.5f, 1.0f,  -1.0f,  0.0f,  0.0f, 1.0f,
       -0.5f, -0.5f, -0.5f, 1.0f,  -1.0f,  0.0f,  0.0f, 1.0f,
       -0.5f, -0.5f,  0.5f, 1.0f,  -1.0f,  0.0f,  0.0f, 1.0f,
       -0.5f,  0.5f,  0.5f, 1.0f,  -1.0f,  0.0f,  0.0f, 1.0f,
        0.5f,  0.5f,  0.5f, 1.0f,   1.0f,  0.0f,  0.0f, 1.0f,
        0.5f,  0.5f, -0.5f, 1.0f,   1.0f,  0.0f,  0.0f, 1.0f,
        0.5f, -0.5f, -0.5f, 1.0f,   1.0f,  0.0f,  0.0f, 1.0f,
        0.5f, -0.5f, -0.5f, 1.0f,   1.0f,  0.0f,  0.0f, 1.0f,
        0.5f, -0.5f,  0.5f, 1.0f,   1.0f,  0.0f,  0.0f, 1.0f,
        0.5f,  0.5f,  0.5f, 1.0f,   1.0f,  0.0f,  0.0f, 1.0f,
       -0.5f, -0.5f, -0.5f, 1.0f,   0.0f, -1.0f,  0.0f, 1.0f,
        0.5f, -0.5f, -0.5f, 1.0f,   0.0f, -1.0f,  0.0f, 1.0f,
        0.5f, -0.5f,  0.5f, 1.0f,   0.0f, -1.0f,  0.0f, 1.0f,
        0.5f, -0.5f,  0.5f, 1.0f,   0.0f, -1.0f,  0.0f, 1.0f,
       -0.5f, -0.5f,  0.5f, 1.0f,   0.0f, -1.0f,  0.0f, 1.0f,
       -0.5f, -0.5f, -0.5f, 1.0f,   0.0f, -1.0f,  0.0f, 1.0f,
       -0.5f,  0.5f, -0.5f, 1.0f,   0.0f,  1.0f,  0.0f, 1.0f,
        0.5f,  0.5f, -0.5f, 1.0f,   0.0f,  1.0f,  0.0f, 1.0f,
        0.5f,  0.5f,  0.5f, 1.0f,   0.0f,  1.0f,  0.0f, 1.0f,
        0.5f,  0.5f,  0.5f, 1.0f,   0.0f,  1.0f,  0.0f, 1.0f,
       -0.5f,  0.5f,  0.5f, 1.0f,   0.0f,  1.0f,  0.0f, 1.0f,
       -0.5f,  0.5f, -0.5f, 1.0f,   0.0f,  1.0f,  0.0f, 1.0f
        };

        unsigned int indicies[] = {
            0 ,
            1 ,
            2 ,
            3 ,
            4 ,
            5 ,
            6 ,
            7 ,
            8 ,
            9 ,
            10,
            11,
            12,
            13,
            14,
            15,
            16,
            17,
            18,
            19,
            20,
            21,
            22,
            23,
            24,
            25,
            26,
            27,
            28,
            29,
            30,
            31,
            32,
            33,
            34,
            35
        };



        auto command_list = Renderer::Get()->GetRenderCommandList();
        auto command_queue = Renderer::Get()->GetCommandQueue();

        GraphicsPipelineDescriptor pipeline_desc;
        pipeline_desc.flags = PipelineFlags::ENABLE_DEPTH_TEST;
        pipeline_desc.layout = VertexLayoutFactory<BoxPreset>::GetLayout();
        pipeline_desc.scissor_rect = RenderScissorRect();
        pipeline_desc.viewport = RenderViewport();
        pipeline_desc.shader = ShaderManager::Get()->GetShader("shaders/BoxShader.glsl");

        pipeline = PipelineManager::Get()->CreatePipeline(pipeline_desc);

        GraphicsPipelineDescriptor pipeline_desc_wireframe;
        pipeline_desc_wireframe.flags = PipelineFlags::ENABLE_DEPTH_TEST;
        pipeline_desc_wireframe.layout = VertexLayoutFactory<BoxPreset>::GetLayout();
        pipeline_desc_wireframe.scissor_rect = RenderScissorRect();
        pipeline_desc_wireframe.viewport = RenderViewport();
        pipeline_desc_wireframe.shader = ShaderManager::Get()->GetShader("shaders/BoxShader.glsl");
        pipeline_desc_wireframe.polygon_render_mode = PrimitivePolygonRenderMode::WIREFRAME;

        pipeline_wireframe = PipelineManager::Get()->CreatePipeline(pipeline_desc_wireframe);

        RenderBufferDescriptor vertex_buffer_desc(sizeof(vertices), RenderBufferType::DEFAULT, RenderBufferUsage::VERTEX_BUFFER);

        vertex_buffer = RenderResourceManager::Get()->CreateBuffer(vertex_buffer_desc);
        RenderResourceManager::Get()->UploadDataToBuffer(command_list, vertex_buffer, vertices, sizeof(vertices), 0);

        RenderBufferDescriptor index_buffer_desc(sizeof(indicies), RenderBufferType::DEFAULT, RenderBufferUsage::INDEX_BUFFER);

        index_buffer = RenderResourceManager::Get()->CreateBuffer(index_buffer_desc);
        RenderResourceManager::Get()->UploadDataToBuffer(command_list, index_buffer, indicies, sizeof(indicies), 0);


        Constant_buffer_type constant_buf_data = {
            camera.GetProjectionMatrix() * camera_transform,
            glm::mat4(1.0f),
            glm::normalize(glm::vec4(0.20f, 1.0f, -3.0f,0.0f)),
            glm::vec4(0.3f,0.7f,1.0f,1.0f),
            glm::vec4(0,0,0,0)
        };
        RenderBufferDescriptor constant_buffer_desc(sizeof(constant_buf_data), RenderBufferType::DEFAULT, RenderBufferUsage::CONSTANT_BUFFER);

        constant_buffer = RenderResourceManager::Get()->CreateBuffer(constant_buffer_desc);
        RenderResourceManager::Get()->UploadDataToBuffer(command_list, constant_buffer, &constant_buf_data, sizeof(constant_buf_data), 0);

        command_queue->ExecuteRenderCommandList(command_list);
    };

    ~Render_Box_data() {
        
    }
    
    std::shared_ptr<Pipeline> pipeline; ///< The rendering pipeline used to render a solid box
    std::shared_ptr<Pipeline> pipeline_wireframe; ///< The rendering pipeline used to render a wireframe box
    std::shared_ptr<RenderBufferResource> vertex_buffer; ///< the buffer containg the box verticies 
    std::shared_ptr<RenderBufferResource> index_buffer; ///< the buffer contaning indicies of the the verticies in the vertex buffer to create the primitives
    std::shared_ptr<RenderBufferResource> constant_buffer; ///< the constant data needed by the shader defined by Render_Box_data::Constant_buffer_type
};

/**
 * @brief Used to get a singleton instance of Render_Box_data used by the Render_Box system
 * @param shutdown if the first time this function is accessed is at deletion there is no need to create the Render_Box_data and instead returns a nullpointer
 * @return if this is the first call and shutdown is true, then nullptr, otherwise an instance of Render_Box_data
*/
inline Render_Box_data* Get_Render_Box_data(bool shutdown = false) {
    
    static Render_Box_data* data = shutdown ? nullptr : new Render_Box_data(CameraComponent(45.0f, 0.1f, 1000.0f, 1.0f),
        glm::lookAt(glm::vec3(1.0f, 1.5f, -5.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)));
    return data;
}

/**
 * @brief Deletes the Render_Box_data 
*/
inline void Delete_Render_Box_data() {
    Render_Box_data* ptr = Get_Render_Box_data(true);
    if (ptr) {
        ptr->clear();
        delete ptr;
    }
}

/**
 * @brief A system used to Draw a box in the scene
 * 
 * @deprecated Only used for drawing debugging visualization elements, otherwise this is just an artifact from the early stages of the Engines development, before a proper
 * rendering pipeline was implemented
 * @param box the bounding box object defining the box to render
 * @param model_matrix the world transformation of the box
 * @param camera the camera component contaning the necessary projection information used for creating the projection matrix
 * @param camera_transform the camera transform used for creating the view matrix
 * @param render_mode defined whether the box should be solid or wireframe
 * @param color defined the color of the box to be rendered
*/
inline void Render_Box(const BoundingBox& box, const glm::mat4& model_matrix,const CameraComponent& camera, const glm::mat4& camera_transform, 
    PrimitivePolygonRenderMode render_mode = PrimitivePolygonRenderMode::DEFAULT, const glm::vec3& color = glm::vec3(0.3f, 0.7f, 1.0f)) {
    auto command_list = Renderer::Get()->GetRenderCommandList();
    auto command_queue = Renderer::Get()->GetCommandQueue();
	
    Render_Box_data& data = *Get_Render_Box_data();
    Render_Box_data::Constant_buffer_type buffer = {
         camera.GetProjectionMatrix() * glm::inverse(camera_transform) * model_matrix * glm::translate(glm::mat4(1.0f), box.GetBoxOffset()) * glm::scale(glm::mat4(1.0f),box.GetBoxSize()),
         model_matrix,
            glm::normalize(glm::vec4(0.20f, 1.0f, -3.0f,0.0f)),
            glm::vec4(color,1.0f),
            glm::vec4(render_mode == PrimitivePolygonRenderMode::WIREFRAME ? 1.0 : 0.0,0,0,0)
    };

    RenderResourceManager::Get()->UploadDataToBuffer(command_list, data.constant_buffer, &buffer, sizeof(buffer), 0);

    command_list->SetDefaultRenderTarget();
    switch (render_mode) {
    case PrimitivePolygonRenderMode::DEFAULT:
        command_list->SetPipeline(data.pipeline);
        break;
    case PrimitivePolygonRenderMode::WIREFRAME:
        command_list->SetPipeline(data.pipeline_wireframe);
        break;
    }
    command_list->SetVertexBuffer(data.vertex_buffer);
    command_list->SetIndexBuffer(data.index_buffer);
    command_list->SetConstantBuffer("conf", data.constant_buffer);
    command_list->Draw(36);

    command_queue->ExecuteRenderCommandList(command_list);

}

/**
 * @brief A debugging visualizer used for rendering the bounding boxes of all Entities which have one
 * @param world the world to visualize the bounding volumes of
 * 
 * @warning Currently only bounding boxes are displayed other types of bounding volumes are not used rendered
*/
inline void BoundingVolumeRender(World& world) {
    auto update = [&world](ComponentCollection compcol, system_view_type<BoundingVolumeComponent>& comps, entt::registry* reg) {
        for (auto& entity : comps) {
            BoundingVolumeComponent& box = world.GetComponent<BoundingVolumeComponent>(entity);
            TransformComponent& transform = world.GetComponent<TransformComponent>(entity);
            std::visit(Overload{
                [&transform,&world](const BoundingBox& box) {
                    CameraComponent& camera_comp = world.GetComponent<CameraComponent>(world.GetPrimaryEntity());
                    TransformComponent& camera_trans = world.GetComponent<TransformComponent>(world.GetPrimaryEntity());
                    Render_Box(box, transform.TransformMatrix, camera_comp, camera_trans.TransformMatrix);
                },
                [&transform,&world](const auto& everything) {

                }

                }, BoundingVolumeComponent::GetBoundingVolume(entity));
        }
    };

    RunSystemSimple<BoundingVolumeComponent>(world, update);

}
#include "TextureManager.h"
#include "TextureManager.h"
#include <Renderer/MeshManager.h>
#include <Renderer/RenderResourceManager.h>
#include <dependencies/stb_image/stb_image.h>
#include <Application.h>
#include <fstream>
#include <FileManager.h>
#include <Renderer/PipelineManager.h>



struct TextureManager_internal {
    std::shared_ptr<Pipeline> reflection_convert_pipeline;
    std::shared_ptr<RenderBufferResource> const_buffer;
    std::shared_ptr<RenderFrameBufferResource> framebuffer;
    std::shared_ptr<RenderTexture2DCubemapResource> default_cubemap;
    std::shared_ptr<RenderTexture2DCubemapResource> default_cubemap_diffuse;
    
    std::shared_ptr<Pipeline> specular_generation_pipeline;
    std::shared_ptr<RenderBufferResource> const_buffer_specular;
    std::shared_ptr<TextureSampler> reflection_sampler;
};



TextureManager* TextureManager::instance = nullptr;

void TextureManager::MakeTextureFromImage(const std::string& input_image_path, const std::string& output_image_path, const TextureSamplerDescritor& sampler)
{
    PROFILE("Texture Conversion");
    if (!stbi_is_hdr(input_image_path.c_str())) {

        int x, y, ch;
        unsigned char* image = stbi_load(input_image_path.c_str(), &x, &y, &ch, 0);
        if (!image) {
            throw std::runtime_error(stbi_failure_reason());
        }

        std::ofstream output_file(output_image_path, std::ios_base::binary);
        if (!output_file.is_open()) {
            throw std::runtime_error("File " + output_image_path + " could not be created");
        }
        auto desc = sampler;

        output_file << "texture_info\n";
        output_file << x << " " << y << " " << ch << "\n";
        output_file << "ldr\n";
        output_file << "sampler\n";
        output_file << (int)desc.AddressMode_U << " " << (int)desc.AddressMode_V << " " << (int)desc.AddressMode_W << "\n";
        output_file << desc.border_color.r << " " << desc.border_color.g << " " << desc.border_color.b << "\n";
        output_file << (int)desc.filter << "\n";
        output_file << desc.LOD_bias << "\n";
        output_file << desc.max_LOD << "\n";
        output_file << desc.min_LOD << "\n";

        output_file << "texture\n";
        output_file.write(const_cast<const char*>((char*)image), x * y * ch);
        output_file << "\nend";

        output_file.close();

        stbi_image_free(image);
    }
    else {
        int x, y, ch;
        float* image = stbi_loadf(input_image_path.c_str(), &x, &y, &ch, 0);
        if (!image) {
            throw std::runtime_error(stbi_failure_reason());
        }

        std::ofstream output_file(output_image_path, std::ios_base::binary);
        if (!output_file.is_open()) {
            throw std::runtime_error("File " + output_image_path + " could not be created");
        }
        auto desc = sampler;

        output_file << "texture_info\n";
        output_file << x << " " << y << " " << ch << "\n";
        output_file << "hdr\n";
        output_file << "sampler\n";
        output_file << (int)desc.AddressMode_U << " " << (int)desc.AddressMode_V << " " << (int)desc.AddressMode_W << "\n";
        output_file << desc.border_color.r << " " << desc.border_color.g << " " << desc.border_color.b << "\n";
        output_file << (int)desc.filter << "\n";
        output_file << desc.LOD_bias << "\n";
        output_file << desc.max_LOD << "\n";
        output_file << desc.min_LOD << "\n";

        output_file << "texture\n";
        output_file.write(const_cast<const char*>((char*)image), x * y * ch * sizeof(float));
        output_file << "\nend";

        output_file.close();

        stbi_image_free(image);
    }

}

Future<void> TextureManager::MakeTextureFromImageAsync(const std::string& input_image_path, const std::string& output_image_path, const TextureSamplerDescritor& sampler)
{
    auto async_queue = Application::GetAsyncDispather();

    auto task = async_queue->CreateTask<void>([input_image_path, output_image_path, sampler, this]() -> void {
        MakeTextureFromImage(input_image_path, output_image_path, sampler);
        return;
        });

    async_queue->Submit(task);

    return task->GetFuture();
}

std::shared_ptr<RenderTexture2DResource> TextureManager::LoadTextureFromFile(const std::string& file_path_in, bool generate_mips)
{
    std::string file_path = FileManager::Get()->GetPath(file_path_in);

    PROFILE("Texture Load");
    //Check if the texture has already been loaded.
    std::unique_lock<std::mutex> lock(texture_Map_mutex);
    auto fnd = texture_Map.find(file_path); //TODO: Make sure the file path is in an appropriate format
    if (fnd != texture_Map.end()) {
        return fnd->second;
    }
    lock.unlock();

    bool is_hdr = false;

    //If it hasn't been loaded open a filestream of the filepath to load it.
    std::ifstream input_file(file_path, std::ios_base::binary | std::ios_base::in);
    if (!input_file.is_open()) {
        throw std::runtime_error("File " + file_path + " could not be opened");
    }

    texture_data texture;
    std::string check;

    //Parse the stread into texture_data structure.
    input_file >> check;
    if (check != "texture_info") throw std::runtime_error("Invalid format on file: " + file_path);
    input_file >> texture.res_x >> texture.res_y >> texture.channels;
    input_file >> check;
    if (check == "hdr") {
        is_hdr = true;
    }

    input_file >> check;
    if (check != "sampler") throw std::runtime_error("Invalid format on file: " + file_path);
    int u, v, w;
    input_file >> u >> v >> w;
    texture.sampler_desc.AddressMode_U = (TextureAddressMode)(char)u;
    texture.sampler_desc.AddressMode_V = (TextureAddressMode)(char)v;
    texture.sampler_desc.AddressMode_W = (TextureAddressMode)(char)w;

    input_file >> texture.sampler_desc.border_color.r >> texture.sampler_desc.border_color.g >> texture.sampler_desc.border_color.b;
    int filter;
    input_file >> filter;
    texture.sampler_desc.filter = (TextureFilter)(char)filter;

    input_file >> texture.sampler_desc.LOD_bias;
    input_file >> texture.sampler_desc.max_LOD;
    input_file >> texture.sampler_desc.min_LOD;

    input_file >> check;
    if (check != "texture") throw std::runtime_error("Invalid format on file: " + file_path);
    if (!is_hdr) {

        int size = texture.channels * texture.res_x * texture.res_y;
        char* data = new char[size];
        input_file.get();
        input_file.read(data, size);
        input_file >> check;
        if (check != "end") throw std::runtime_error("Invalid format on file: " + file_path);
        texture.tex_data = (unsigned char*)data;
    }
    else {
        int size = texture.channels * texture.res_x * texture.res_y * sizeof(float);
        char* data = new char[size];
        input_file.get();
        input_file.read(data, size);
        input_file >> check;
        if (check != "end") throw std::runtime_error("Invalid format on file: " + file_path);
        texture.tex_data = (unsigned char*)data;
    }
    
    //Check if an identical sampler already exists. If not Create it.
    std::unique_lock<std::mutex> lock2(sampler_cache_mutex);
    std::shared_ptr<TextureSampler> sampler;
    auto sampler_iter = sampler_cache.find(texture.sampler_desc);
    if (sampler_iter == sampler_cache.end()) {
        sampler = sampler_cache.insert(std::make_pair(texture.sampler_desc,TextureSampler::CreateSampler(texture.sampler_desc))).first->second;
    }
    else {
        sampler = sampler_iter->second;
    }
    lock2.unlock();

    //Create the texture, upload data into it and optionally generate mip maps.
    RenderTexture2DDescriptor texture_desc;
    if (!is_hdr) {
        texture_desc.format = texture.channels == 3 ? TextureFormat::RGB_UNSIGNED_CHAR : TextureFormat::RGBA_UNSIGNED_CHAR;
    }
    else {
        texture_desc.format = TextureFormat::RGB_32FLOAT;
    }
    texture_desc.height = texture.res_y;
    texture_desc.width = texture.res_x;
    texture_desc.sampler = sampler;

    auto command_list = Renderer::Get()->GetRenderCommandList(); //TODO:Make sure you dont use too many command lists.
    auto command_queue = Renderer::Get()->GetCommandQueue();

    std::shared_ptr<RenderTexture2DResource> texture_res = RenderResourceManager::Get()->CreateTexture(texture_desc);
    RenderResourceManager::Get()->UploadDataToTexture2D(command_list, texture_res, texture.tex_data, texture.res_x, texture.res_y, 0, 0, 0);
    if(generate_mips) command_list->GenerateMIPs(texture_res);

    command_queue->ExecuteRenderCommandList(command_list);

    lock.lock();

    //Cache the loaded texture.
    auto texture_final = texture_Map.insert(std::make_pair(file_path, texture_res));

    return texture_final.first->second; 

}

Future<std::shared_ptr<RenderTexture2DResource>> TextureManager::LoadTextureFromFileAsync(const std::string& file_path, bool generate_mips)
{
    auto async_queue = Application::GetAsyncDispather();

    auto task = async_queue->CreateTask<std::shared_ptr<RenderTexture2DResource>>([file_path, generate_mips,this]() -> std::shared_ptr<RenderTexture2DResource> {
        return LoadTextureFromFile(file_path, generate_mips);
        });

    async_queue->Submit(task);

    return task->GetFuture();

}

bool TextureManager::IsTextureAvailable(const std::string& file_path_in)
{
    std::string file_path = FileManager::Get()->GetPath(file_path_in);
    std::lock_guard<std::mutex> lock(texture_Map_mutex);
    auto fnd = texture_Map.find(file_path); //TODO: Make sure the file path is in an appropriate format
    return fnd != texture_Map.end();
}

void TextureManager::ReleaseTexture(const std::string& file_path_in)
{
    std::string file_path = FileManager::Get()->GetPath(file_path_in);
    std::lock_guard<std::mutex> lock(texture_Map_mutex);
    texture_Map.erase(file_path);

}

std::shared_ptr<ReflectionMap> TextureManager::GetReflectionMap(const std::string& path_in)
{
    auto path = FileManager::Get()->GetPath(path_in);
    std::unique_lock<std::mutex> lock(reflection_maps_mutex);
    auto fnd = reflection_maps.find(path);
    if (fnd != reflection_maps.end()) {
        return fnd->second;
    }
    auto result = std::make_shared<ReflectionMap>();
    result->status = ReflectionMapStatus::LOADING;
    reflection_maps.insert(std::make_pair(path, result));

    auto async_queue = Application::GetAsyncDispather();
    auto cube_mesh = MeshManager::Get()->LoadMeshFromFileAsync("asset:Box.mesh"_path);
    //CubeMesh is loaded before submitting the task to guarantee the mesh will be ready by the time this task runs.
    auto task = async_queue->CreateTask<ReflectionMap>([path_in,cube_mesh, this]() -> ReflectionMap {
        auto base_texture = LoadTextureFromFile(path_in);
        ReflectionMap result;
        RenderTexture2DCubemapDescriptor cb_desc;
        cb_desc.format = TextureFormat::RGB_32FLOAT;
        cb_desc.sampler = data->default_cubemap->GetBufferDescriptor().sampler;
        cb_desc.res = REFLECTION_RES;

        std::shared_ptr<RenderTexture2DCubemapResource> converted_cubemap = RenderResourceManager::Get()->CreateTextureCubemap(cb_desc);
        std::shared_ptr<RenderTexture2DCubemapResource> converted_cubemap_diffuse = RenderResourceManager::Get()->CreateTextureCubemap(cb_desc);
        cb_desc.res = SPECULAR_REFLECTION_RES;
        cb_desc.generate_mips = true;
        cb_desc.sampler = data->reflection_sampler;
        std::shared_ptr<RenderTexture2DCubemapResource> converted_cubemap_specular = RenderResourceManager::Get()->CreateTextureCubemap(cb_desc);

        auto list = Renderer::Get()->GetRenderCommandList();
        auto queue = Renderer::Get()->GetCommandQueue();


        glm::mat4 light_views[6];
        glm::mat4 projection = glm::perspective(glm::radians(90.0f), 1.0f, 0.01f, 1000.0f);
        light_views[0] = projection * glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f) + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
        light_views[1] = projection * glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f) + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
        light_views[2] = projection * glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f) + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        light_views[3] = projection * glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f) + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f));
        light_views[4] = projection * glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f) + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
        light_views[5] = projection * glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f) + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
        RenderResourceManager::Get()->UploadDataToBuffer(list, data->const_buffer, glm::value_ptr(light_views[0]), sizeof(glm::mat4) * 6, 0);
        RenderResourceManager::Get()->SetFrameBufferColorAttachment(list, data->framebuffer, converted_cubemap);
        RenderResourceManager::Get()->SetFrameBufferColorAttachment(list,data->framebuffer, converted_cubemap_diffuse, 1);
        list->SetPipeline(data->reflection_convert_pipeline);
        list->SetRenderTarget(data->framebuffer);
        list->SetViewport(RenderViewport(glm::vec2(0.0f), glm::vec2(800, 800)));
        list->SetConstantBuffer("mvp", data->const_buffer);
        list->SetTexture2D("in_tex", base_texture);
        list->SetVertexBuffer(cube_mesh->GetVertexBuffer());
        list->SetIndexBuffer(cube_mesh->GetIndexBuffer());
        list->Draw(cube_mesh->GetIndexCount());
        
        RenderResourceManager::Get()->UploadDataToBuffer(list, data->const_buffer_specular, glm::value_ptr(light_views[0]), sizeof(glm::mat4) * 6, 0);
        RenderResourceManager::Get()->SetFrameBufferColorAttachment(list, data->framebuffer, data->default_cubemap_diffuse, 1);
        int i = 0;
        for (float roughness = 0.0; roughness <= 1.0; roughness += 0.2f) {
            RenderResourceManager::Get()->SetFrameBufferColorAttachment(list, data->framebuffer, converted_cubemap_specular, 0,i);
            RenderResourceManager::Get()->UploadDataToBuffer(list, data->const_buffer_specular, &roughness, sizeof(float), sizeof(glm::mat4) * 6);
            
            unsigned int res = SPECULAR_REFLECTION_RES * std::pow(0.5, i);
            list->SetPipeline(data->specular_generation_pipeline);
            list->SetRenderTarget(data->framebuffer);
            list->SetViewport(RenderViewport(glm::vec2(0.0f), glm::vec2(res, res)));
            list->SetConstantBuffer("mvp", data->const_buffer_specular);
            list->SetTexture2DCubemap("in_tex", converted_cubemap);
            list->SetVertexBuffer(cube_mesh->GetVertexBuffer());
            list->SetIndexBuffer(cube_mesh->GetIndexBuffer());
            list->Draw(cube_mesh->GetIndexCount());
            i++;
        }


        queue->ExecuteRenderCommandList(list);

        result.specular_map = converted_cubemap_specular;
              
        result.diffuse_map = converted_cubemap_diffuse;
              
        result.status = ReflectionMapStatus::LOADED;

        return result;
        });

    async_queue->Submit(task);


    relection_map_load_future future;
    future.future = task->GetFuture();
    future.reflection_map = result;
    future.path = path;

    std::lock_guard<std::mutex> lock2(reflection_map_Load_queue_mutex);
    reflection_map_Load_queue.push_back(future);

    return result;

    
}

void TextureManager::Init()
{
    if (!instance) {
        instance = new TextureManager;
    }
}

TextureManager* TextureManager::Get()
{
    return instance;
}

void TextureManager::Shutdown()
{
    if (instance) {
        delete instance->data;
        delete instance;
    }
}

TextureManager::TextureManager() : texture_Map(), texture_Map_mutex(), sampler_cache(), sampler_cache_mutex(), reflection_maps(), reflection_maps_mutex(), data(new TextureManager_internal)
{
    RenderTexture2DDescriptor tex_desc;
    tex_desc.format = TextureFormat::RGBA_UNSIGNED_CHAR;
    tex_desc.height = 1;
    tex_desc.width = 1;
    tex_desc.sampler = TextureSampler::CreateSampler(TextureSamplerDescritor());

    RenderTexture2DDescriptor tex_normal;
    tex_normal.format = TextureFormat::RGBA_32FLOAT;
    tex_normal.height = 1;
    tex_normal.width = 1;
    tex_normal.sampler = tex_desc.sampler;

    RenderTexture2DArrayDescriptor tex_array_desc;
    tex_array_desc.format = TextureFormat::RGBA_UNSIGNED_CHAR;
    tex_array_desc.height = 1;
    tex_array_desc.width = 1;
    tex_array_desc.num_of_textures = 1;
    tex_array_desc.sampler = TextureSampler::CreateSampler(TextureSamplerDescritor());

    RenderTexture2DCubemapDescriptor tex_cubemap_desc;
    tex_cubemap_desc.format = TextureFormat::RGBA_UNSIGNED_CHAR;
    tex_cubemap_desc.res = 1;
    tex_cubemap_desc.sampler = TextureSampler::CreateSampler(TextureSamplerDescritor());

    auto def_tex = RenderResourceManager::Get()->CreateTexture(tex_desc);
    auto def_normal_tex = RenderResourceManager::Get()->CreateTexture(tex_normal);
    auto def_tex_arr = RenderResourceManager::Get()->CreateTextureArray(tex_array_desc);
    auto def_tex_cbm = RenderResourceManager::Get()->CreateTextureCubemap(tex_cubemap_desc);
    auto queue = Renderer::Get()->GetCommandQueue();
    auto command_list = Renderer::Get()->GetRenderCommandList();
    unsigned char tex_data[4] = { 255,255,255,255 };
    float normal_tex_data[4] = { 0.5f,0.5f,1.0f,1.0f };
    RenderResourceManager::Get()->UploadDataToTexture2D(command_list, def_tex, &tex_data, 1, 1, 0, 0);
    RenderResourceManager::Get()->UploadDataToTexture2D(command_list, def_normal_tex, &normal_tex_data, 1, 1, 0, 0);
    RenderResourceManager::Get()->UploadDataToTexture2DArray(command_list, def_tex_arr,0, &tex_data, 1, 1, 0, 0);
    for (int i = 0; i < 6; i++) {
        RenderResourceManager::Get()->UploadDataToTexture2DCubemap(command_list, def_tex_cbm, (CubemapFace)((char)CubemapFace::POSITIVE_X + i), &tex_data, 1, 1, 0, 0);
    }
    queue->ExecuteRenderCommandList(command_list);
    default_texture = def_tex;
    default_normal_texture = def_normal_tex;
    default_texture_array = def_tex_arr;
    default_texture_cubemap = def_tex_cbm;

    PipelineDescriptor pipeline_desc;
    pipeline_desc.cull_mode = CullMode::NONE;
    pipeline_desc.enable_depth_clip = false;
    pipeline_desc.shader = ShaderManager::Get()->GetShader("shaders/ReflectionMapConversion.glsl");
    pipeline_desc.layout = VertexLayoutFactory<MeshPreset>::GetLayout();

    data->reflection_convert_pipeline = PipelineManager::Get()->CreatePipeline(pipeline_desc);

    pipeline_desc.shader = ShaderManager::Get()->GetShader("shaders/SpecularMapGeneration.glsl");

    data->specular_generation_pipeline = PipelineManager::Get()->CreatePipeline(pipeline_desc);

    TextureSamplerDescritor reflection_sampler_desc;
    reflection_sampler_desc.filter = TextureFilter::LINEAR_MIN_MAG_MIP;
    data->reflection_sampler = TextureSampler::CreateSampler(reflection_sampler_desc);


    RenderBufferDescriptor const_reflection_buffer_desc(sizeof(glm::mat4)*6,RenderBufferType::UPLOAD, RenderBufferUsage::CONSTANT_BUFFER);

    data->const_buffer = RenderResourceManager::Get()->CreateBuffer(const_reflection_buffer_desc);

    RenderBufferDescriptor specular_generation_buffer_desc(sizeof(glm::mat4) * 6 + sizeof(float), RenderBufferType::UPLOAD, RenderBufferUsage::CONSTANT_BUFFER);

    data->const_buffer_specular = RenderResourceManager::Get()->CreateBuffer(specular_generation_buffer_desc);

    RenderTexture2DCubemapDescriptor default_cubemap;
    default_cubemap.format = TextureFormat::RGB_32FLOAT;
    default_cubemap.res = REFLECTION_RES;
    default_cubemap.sampler = TextureSampler::CreateSampler(TextureSamplerDescritor());
    


    RenderFrameBufferDescriptor frame_desc;
    data->default_cubemap = RenderResourceManager::Get()->CreateTextureCubemap(default_cubemap);
    data->default_cubemap_diffuse = RenderResourceManager::Get()->CreateTextureCubemap(default_cubemap);
    frame_desc.color_attachments.push_back({ 0,data->default_cubemap });
    frame_desc.color_attachments.push_back({ 0,data->default_cubemap_diffuse });
    data->framebuffer = RenderResourceManager::Get()->CreateFrameBuffer(frame_desc);



}

void TextureManager::ClearTextureCache()
{
    std::lock_guard<std::mutex> lock1(sampler_cache_mutex);
    std::lock_guard<std::mutex> lock2(texture_Map_mutex);

    texture_Map.clear();
    sampler_cache.clear();
}

void TextureManager::UpdateLoadedReflectionMaps()
{
    std::lock_guard<std::mutex> lock(reflection_map_Load_queue_mutex);
    for (auto& loaded_reflection_map : reflection_map_Load_queue) {
        if (!loaded_reflection_map.future.IsAvailable() || loaded_reflection_map.destroyed) continue;
        try {
            *(loaded_reflection_map.reflection_map) = std::move(loaded_reflection_map.future.GetValue());

            loaded_reflection_map.destroyed = true;
        }
        catch (...) {
            loaded_reflection_map.reflection_map->status = ReflectionMapStatus::ERROR;
            std::lock_guard<std::mutex> lock(reflection_maps_mutex);
            reflection_maps.erase(loaded_reflection_map.path);
            loaded_reflection_map.destroyed = true;
        }
    }


    while (!reflection_map_Load_queue.empty() && reflection_map_Load_queue.front().destroyed) {
        reflection_map_Load_queue.pop_front();
    }
}

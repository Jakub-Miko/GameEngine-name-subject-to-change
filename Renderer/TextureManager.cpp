#include "TextureManager.h"
#include "TextureManager.h"
#include <Renderer/RenderResourceManager.h>
#include <dependencies/stb_image/stb_image.h>
#include <Application.h>
#include <fstream>
#include <FileManager.h>

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
        delete instance;
    }
}

TextureManager::TextureManager() : texture_Map(), texture_Map_mutex(), sampler_cache(), sampler_cache_mutex()
{
    RenderTexture2DDescriptor tex_desc;
    tex_desc.format = TextureFormat::RGBA_UNSIGNED_CHAR;
    tex_desc.height = 1;
    tex_desc.width = 1;
    tex_desc.sampler = TextureSampler::CreateSampler(TextureSamplerDescritor());

    auto def_tex = RenderResourceManager::Get()->CreateTexture(tex_desc);
    auto queue = Renderer::Get()->GetCommandQueue();
    auto command_list = Renderer::Get()->GetRenderCommandList();
    unsigned char tex_data[4] = { 255,255,255,255 };
    RenderResourceManager::Get()->UploadDataToTexture2D(command_list, def_tex, &tex_data, 1, 1, 0, 0);
    queue->ExecuteRenderCommandList(command_list);
    default_texture = def_tex;
}

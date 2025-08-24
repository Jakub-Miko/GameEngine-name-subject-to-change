#pragma once
#include <Renderer/Renderer3D/Renderer3D.h>
#include <variant>
#include <deque>
#include <AsyncTaskDispatcher.h>
#include <Core/FrameMultiBufferResource.h>

class Shader;
class RenderCommandList;

class Material;

struct MaterialTextureType {
    std::shared_ptr<RenderTexture2DResource> texture;
#ifdef EDITOR
    std::string path = "";
#endif
};

using MaterialParameterResource = std::variant<int, float, glm::vec2, glm::vec3, glm::vec4, glm::mat4, glm::mat3, MaterialTextureType, std::shared_ptr<RenderTexture2DArrayResource>, 
    std::shared_ptr<RenderTexture2DCubemapResource>, std::shared_ptr<RenderBufferResource>, std::string, std::monostate>;

enum class MaterialLayoutItemType : char {
    SCALAR = 0, VEC2 = 1, VEC3 = 2, VEC4 = 3, TEXTURE = 4, MAT3 = 5, MAT4 = 6, INT = 7, TEXTURE_2D_ARRAY = 8, TEXTURE_2D_CUBEMAP = 9, CONSTANT_BUFFER = 10,  INVALID_PARAMETER = -1
};

struct MaterialLayoutItem {
    std::string name;
    MaterialLayoutItemType type = MaterialLayoutItemType::INVALID_PARAMETER;
    MaterialParameterResource default_value = std::monostate();
    union { // This is filled in by the constructor of the RenderDescriptorHeap, which handles allocations and layouts
        uint32_t set_binding;
        uint32_t constant_buffer_offset;
    };
};

struct MaterialLayout {
    std::vector<MaterialLayoutItem> layout_items;
    uint32_t const_buffer_size = 0; //0 if not constant buffer exists
};

class MaterialTemplate : public std::enable_shared_from_this<MaterialTemplate> {
    struct Private {}; //Used to ensure constructor is not called outside the CreateTemplate function, without making it private(since make_shared needs it)
public:
    MaterialTemplate();

    MaterialTemplate(const MaterialTemplate& other) : material_parameters(other.material_parameters), material_parameters_map(material_parameters_map) {}
    MaterialTemplate& operator=(const MaterialTemplate& other) {;
        material_parameters = other.material_parameters;
        material_parameters_map = other.material_parameters_map;
        return *this;
    }

    ~MaterialTemplate();

    static std::shared_ptr<MaterialTemplate> CreateTemplate(const MaterialLayout& layout, std::string name);

    MaterialTemplate(const MaterialLayout& layout, std::string name, Private dummy);

    const MaterialLayoutItem& GetMaterialTemplateParameter(const std::string& name) const;

    int GetMaterialTemplateParameterIndex(const std::string& name) const;

    const MaterialLayout& GetMaterialTemplateParameters() const {
        return material_parameters;
    }

    RenderDescriptorAllocationHandle AllocateMaterialDescriptor();

    std::shared_ptr<Material> GetDefaultMaterial() {
        return default_material;
    }

    const std::string& GetName() const { return material_name; }

    RenderDescriptorHeap& GetAllocator()  {
        return *material_allocator;
    }
   

private:

    std::string material_name = "";
    std::unique_ptr<RenderDescriptorHeap> material_allocator;
    std::shared_ptr<Material> default_material;
    MaterialLayout material_parameters;
    std::unordered_map<std::string, size_t> material_parameters_map;
};

class Material : public std::enable_shared_from_this<Material> {
public:
    Material() = default;
    Material(std::shared_ptr<MaterialTemplate> material_template);

    using material_resource_type = std::variant<FrameMultiBufferResource<RenderDescriptorTable>,std::shared_ptr<RenderBufferResource>>;
    
    enum class Material_status : char {
        OK = 0, ERROR = 1, UNINITIALIZED = 2
    };

    enum class MaterialParameter_flags : char {
        DIRTY = 1,
        DEFAULT = 2
    };

    struct MaterialParameter {
        MaterialParameterResource resource;
        std::string name;
        MaterialLayoutItemType type;
        MaterialParameter_flags flags = MaterialParameter_flags(0);

        bool IsDirty() const;

    };

    void SetMaterial(std::shared_ptr<RenderCommandList>  command_list);

    void ActivateParameter(const std::string& name);
    void DeactivateParameter(const std::string& name);

    template<typename T>
    void SetParameter(const std::string& name, T value);

    void SetParameter(const std::string& name, std::shared_ptr<RenderTexture2DResource> value);

    void SetParameter(const std::string& name, std::shared_ptr<RenderTexture2DResource> value, const std::string path);

    void SetTexture(const std::string& name, const std::string& path);

    Material_status GetStatus() const {
        return status;
    }

    const std::string& GetFilePath() const {
        return material_path;
    }

    const std::vector<MaterialParameter>& GetMaterialParameters() const {
        return parameters;
    }

    std::shared_ptr<RenderBufferResource> GetConstantBuffer() {
        return constant_buffer;
    }


    std::shared_ptr<MaterialTemplate> GetMaterialTemplate() const {
        if(auto ptr = material_template.lock()) {
            return ptr;
        } else {
            throw std::runtime_error("The material template was unloaded but a material attempted to use it.\n");
        }
        
    }

    void UpdateValues(std::shared_ptr<RenderCommandList>  command_list);

private:

    void SetParameterTypeDefault(MaterialParameter& param);
    friend class MaterialManager;
    friend class RenderCommandList;
#ifdef EDITOR
    friend class MaterialEditor;
#endif

    std::string material_path = "";
    std::weak_ptr<MaterialTemplate> material_template;
    std::vector<MaterialParameter> parameters = std::vector<MaterialParameter>();
    RenderDescriptorAllocationHandle descriptor_table;
    std::shared_ptr<RenderBufferResource> constant_buffer = nullptr;
    Material_status status = Material_status::ERROR;
};

NonIntrusiveRuntimeTag(std::shared_ptr<Material>, "Material");

inline Material::MaterialParameter_flags operator|(const Material::MaterialParameter_flags& first, const Material::MaterialParameter_flags& second) {
    return Material::MaterialParameter_flags((char)first | (char)second);
}

inline Material::MaterialParameter_flags operator&(const Material::MaterialParameter_flags& first, const Material::MaterialParameter_flags& second) {
    return Material::MaterialParameter_flags((char)first & (char)second);
}

inline Material::MaterialParameter_flags& operator|=(Material::MaterialParameter_flags& first, Material::MaterialParameter_flags second) {
    return first = first | second;
}

inline Material::MaterialParameter_flags& operator&=(Material::MaterialParameter_flags& first, Material::MaterialParameter_flags second) {
    return first = first & second;
}

inline Material::MaterialParameter_flags operator~(Material::MaterialParameter_flags first) {
    return (Material::MaterialParameter_flags)(~(char)first);
}

inline void Material::SetParameter(const std::string& name, std::shared_ptr<RenderTexture2DResource> value, const std::string path) {
    auto& param = parameters[GetMaterialTemplate()->GetMaterialTemplateParameterIndex(name)];
    param.flags |= MaterialParameter_flags::DIRTY;
    param.flags &= ~MaterialParameter_flags::DEFAULT;
    if (param.type != MaterialLayoutItemType::TEXTURE) throw std::runtime_error("Parameter " + name + "assignment type mismatch");
    MaterialTextureType type;
    type.texture = value;
#ifdef EDITOR
    type.path = path;
#endif
    param.resource = type;
}

template<typename T>
inline void Material::SetParameter(const std::string& name, T value) {
    auto& param = parameters[GetMaterialTemplate()->GetMaterialTemplateParameterIndex(name)];
    param.flags |= MaterialParameter_flags::DIRTY;
    param.flags &= ~MaterialParameter_flags::DEFAULT;
    if (!std::holds_alternative<T>(param.resource) && !std::holds_alternative<std::monostate>(param.resource)) throw std::runtime_error("Parameter " + name + "assignment type mismatch");
    param.resource = value;
}


inline void Material::SetParameter(const std::string& name, std::shared_ptr<RenderTexture2DResource> value) {
    auto& param = parameters[GetMaterialTemplate()->GetMaterialTemplateParameterIndex(name)];
    param.flags |= MaterialParameter_flags::DIRTY;
    param.flags &= ~MaterialParameter_flags::DEFAULT;
    if (param.type != MaterialLayoutItemType::TEXTURE) throw std::runtime_error("Parameter " + name + "assignment type mismatch");
    MaterialTextureType type;
    type.texture = value;
    param.resource = type;
}

class MaterialManager {
public:
    MaterialManager(const MaterialManager& ref) = delete;
    MaterialManager(MaterialManager&& ref) = delete;
    MaterialManager& operator=(const MaterialManager& ref) = delete;
    MaterialManager& operator=(MaterialManager&& ref) = delete;

    static void Init();
    static void Shutdown();
    static MaterialManager* Get();

public:

    std::shared_ptr<Material> GetMaterial(const std::string& path);

    std::shared_ptr<Material> CreateMaterial(const std::string& template_path);

    void SerializeMaterial(const std::string& filepath, std::shared_ptr<Material> material);

    std::shared_ptr<Material> CreateEmptyMaterial(const std::string& filepath, std::shared_ptr<MaterialTemplate> shader);

    std::shared_ptr<MaterialTemplate> GetMaterialTemplate(const std::string& name);

    std::shared_ptr<MaterialTemplate> LoadMaterialTemplateFromJson(const nlohmann::json& json_object, const std::string& name = "");

    void LoadMaterialTemplateFile(const std::string& path);

    void RegisterMaterialTemplate(std::shared_ptr<MaterialTemplate> material_template);

private:


private:
    friend class Material;
    friend class Renderer3D;
    friend class ShaderManager;
    friend class World;


    void ClearMaterialCache();

    void UpdateMaterials();
    MaterialManager();
    struct Material_loading_item {
        std::string name;
        std::shared_ptr<Material> material;
        Future<std::shared_ptr<RenderTexture2DResource>> future;
        bool destroyed = false;
#ifdef EDITOR
        std::string path = "";
#endif
    };
    void AddTextureLoad(std::shared_ptr<Material> material, std::string name, Future<std::shared_ptr<RenderTexture2DResource>> future, const std::string path = "");

    std::shared_ptr<Material> ParseMaterialFromFile(const std::string& path);
    std::shared_ptr<Material> ParseMaterialFromString(const std::string& string, std::shared_ptr<MaterialTemplate> material_template = nullptr);

    std::mutex material_mutex;
    std::unordered_map<std::string, std::shared_ptr<MaterialTemplate>> material_templates;
    std::unordered_map<std::string, std::shared_ptr<Material>> materials;
    std::unordered_set<std::string> loaded_signature_files;
    std::mutex material_load_mutex;
    std::deque<Material_loading_item> material_load;
    static MaterialManager* instance;
};
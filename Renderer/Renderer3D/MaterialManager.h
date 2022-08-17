#pragma once
#include <Renderer/Renderer3D/Renderer3D.h>
#include <Renderer/RootSignature.h>
#include <Renderer/Renderer.h>
#include <variant>
#include <deque>
#include <AsyncTaskDispatcher.h>

class Shader;

class MaterialTemplate {
public:
    enum MaterialTemplateParameterType : char {
        SCALAR = 0, VEC2 = 1, VEC3 = 2, VEC4 = 3, TEXTURE = 4, MAT3 = 5, MAT4 = 6, INT = 7, INVALID_PARAMETER = -1
    };
    
    
    struct MaterialTemplateParameter {
        std::string name;
        union {
            uint32_t descriptor_table_id = -1;
            uint32_t primitive_size;
        };
        uint32_t index = -1;
        uint32_t constant_buffer_offset = -1;
        MaterialTemplateParameterType type = INVALID_PARAMETER;
    };

    MaterialTemplate() : material_parameters(), material_parameters_map() {}
    MaterialTemplate(std::shared_ptr<Shader> shader);
    MaterialTemplate(const MaterialTemplate& other) : material_parameters(other.material_parameters), material_parameters_map(material_parameters_map), shader_wk(other.GetShader()) {}
    MaterialTemplate& operator=(const MaterialTemplate& other) {;
        material_parameters = other.material_parameters;
        material_parameters_map = other.material_parameters_map;
        shader_wk = other.GetShader();
        return *this;
    }

    ~MaterialTemplate();

    std::shared_ptr<Shader> GetShader() const {
        return shader_wk.lock();
    }

    const MaterialTemplateParameter& GetMaterialTemplateParameter(const std::string& name) const;

    int GetMaterialTemplateParameterIndex(const std::string& name) const;

    const std::vector<MaterialTemplateParameter>& GetMaterialTemplateParameters() const {
        return material_parameters;
    }

    const RootSignature& GetRootSignature() const;

    int GetTableOrBufferSize(int index);

private:
    void CreateParameter(const MaterialTemplateParameter& parameter);
    void AddTexture2DParameter(const RootSignatureDescriptorElement& element, int index, uint32_t table = -1);
    void AddDescriptorTableParameter(const RootSignatureDescriptorElement& element, int index);
    void AddConstantBufferParameter(const RootSignatureDescriptorElement& element, int index, uint32_t table = -1);
    std::weak_ptr<Shader> shader_wk; // Need to use weak_ptr since default materials are owned by their own shader which causes cyclic references
    std::vector<MaterialTemplateParameter> material_parameters;
    std::unordered_map<int, int> buffer_and_descriptor_table_sizes;
    std::unordered_map<std::string, size_t> material_parameters_map;
};

class Material : public std::enable_shared_from_this<Material>{
public:

    struct Texture_type {
        std::shared_ptr<RenderTexture2DResource> texture;
#ifdef EDITOR
        std::string path = "";
#endif
    };

    using material_parameter_type = std::variant<int, float, glm::vec2, glm::vec3, glm::vec4, Texture_type>;
    using material_resource_type = std::variant<RenderDescriptorTable,std::shared_ptr<RenderBufferResource>>;

    enum class Material_status : char {
        OK = 0, ERROR = 1, UNINITIALIZED = 2
    };

    struct MaterialResource {
        int index = -1;
        material_resource_type resource;
        bool is_table = false;
    };

    enum class MaterialParameter_flags : char {
        DIRTY = 1,
        TABLE = 2,
        DEFAULT = 4
    };

    struct MaterialParameter {
        material_parameter_type resource;
        std::string name;
        MaterialTemplate::MaterialTemplateParameterType type;
        MaterialParameter_flags flags = MaterialParameter_flags(0);

        bool IsDirty() const;

    };

    void SetMaterial(RenderCommandList* command_list, std::shared_ptr<Pipeline> pipeline);

    void ActivateParameter(const std::string& name);
    void DeactivateParameter(const std::string& name);

    template<typename T>
    void SetParameter(const std::string& name, T value);

    template<>
    void SetParameter<std::shared_ptr<RenderTexture2DResource>>(const std::string& name, std::shared_ptr<RenderTexture2DResource> value);

    void SetParameter(const std::string& name, std::shared_ptr<RenderTexture2DResource> value, const std::string path);

    void SetTexture(const std::string& name, const std::string& path);

    Material_status GetStatus() const {
        return status;
    }

    const std::string& GetFilePath() const {
        return material_path;
    }

private:

    void SetParameterTypeDefault(MaterialParameter& param);

    friend class MaterialManager;
#ifdef EDITOR
    friend class MaterialEditor;
#endif
    Material(std::shared_ptr<MaterialTemplate> material_template);
    void UpdateValues(RenderCommandList* command_list);
    std::string material_path;
    std::shared_ptr<MaterialTemplate> material_template;
    std::vector<MaterialParameter> parameters;
    std::vector<MaterialResource> resources;
    Material_status status = Material_status::OK;
};

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
    auto& param = parameters[material_template->GetMaterialTemplateParameterIndex(name)];
    param.flags |= MaterialParameter_flags::DIRTY;
    param.flags &= ~MaterialParameter_flags::DEFAULT;
    if (!std::holds_alternative<Texture_type>(param.resource)) throw std::runtime_error("Parameter " + name + "assignment type mismatch");
    Texture_type type;
    type.texture = value;
#ifdef EDITOR
    type.path = path;
#endif
    param.resource = type;
}

template<typename T>
inline void Material::SetParameter(const std::string& name, T value) {
    auto& param = parameters[material_template->GetMaterialTemplateParameterIndex(name)];
    param.flags |= MaterialParameter_flags::DIRTY;
    param.flags &= ~MaterialParameter_flags::DEFAULT;
    if (!std::holds_alternative<T>(param.resource)) throw std::runtime_error("Parameter " + name + "assignment type mismatch");
    param.resource = value;
}


template<>
inline void Material::SetParameter<std::shared_ptr<RenderTexture2DResource>>(const std::string& name, std::shared_ptr<RenderTexture2DResource> value) {
    auto& param = parameters[material_template->GetMaterialTemplateParameterIndex(name)];
    param.flags |= MaterialParameter_flags::DIRTY;
    param.flags &= ~MaterialParameter_flags::DEFAULT;
    if (!std::holds_alternative<Texture_type>(param.resource)) throw std::runtime_error("Parameter " + name + "assignment type mismatch");
    Texture_type type;
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

    std::shared_ptr<Material> CreateMaterial(const std::string& shader_path);

    void SerializeMaterial(const std::string& filepath, std::shared_ptr<Material> material);

    std::shared_ptr<Material> CreateEmptyMaterial(const std::string& filepath, std::shared_ptr<Shader> shader);

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
    std::shared_ptr<Material> ParseMaterialFromString(const std::string& string, std::shared_ptr<Shader> shader_spec = nullptr);

    std::mutex material_mutex;
    std::unordered_map<std::string, std::shared_ptr<MaterialTemplate>> material_templates;
    std::unordered_map<std::string, std::shared_ptr<Material>> materials;
    std::mutex material_load_mutex;
    std::deque<Material_loading_item> material_load;
    static MaterialManager* instance;
};
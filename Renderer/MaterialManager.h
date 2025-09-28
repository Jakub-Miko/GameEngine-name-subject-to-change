#pragma once
#include <Renderer/Renderer3D/Renderer3D.h>
#include <variant>
#include <deque>
#include <AsyncTaskDispatcher.h>
#include <Core/FrameMultiBufferResource.h>
#include <Renderer/TextureManager.h>

class Shader;
class RenderCommandList;

class Material;

class MaterialProxy {
public:
    virtual std::shared_ptr<Material> LoadMaterial() = 0;
    virtual const std::string& GetFilePath() = 0;
    virtual const std::string& GetNativeFilePath() = 0;
    virtual ~MaterialProxy() {}
}; 

class NativeMaterialProxy : public MaterialProxy {
public: 
    NativeMaterialProxy(const std::string& path) : path(path) {}
    virtual std::shared_ptr<Material> LoadMaterial() override;
    virtual const std::string& GetFilePath() override {
        return path;
    }
    virtual const std::string& GetNativeFilePath() override {
        return path;
    }
    virtual ~NativeMaterialProxy() {}
private:
    std::string path;
}; 

struct MaterialTextureType {
    std::string GetPath() {
        if(texture_proxy) {
            return texture_proxy->GetFilePath();
        }
        return "";
    };

    std::string GetNativePath() {
        if(texture_proxy) {
            return texture_proxy->GetNativeFilePath();
        }
        return "";
    };
    std::shared_ptr<RenderTexture2DResource> texture = nullptr;
    std::shared_ptr<TextureProxy> texture_proxy = nullptr;
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
public:
    MaterialTemplate() = delete;
    MaterialTemplate(const MaterialTemplate& other) = delete;
    MaterialTemplate& operator=(const MaterialTemplate& other) = delete;
    
    virtual ~MaterialTemplate() {};
    
    static std::shared_ptr<MaterialTemplate> CreateTemplate(const MaterialLayout& layout, std::string name);
    
    virtual std::shared_ptr<Material> CreateMaterial() = 0;
    
    const MaterialLayoutItem& GetMaterialTemplateParameter(const std::string& name) const;
    
    int GetMaterialTemplateParameterIndex(const std::string& name) const;
    
    const MaterialLayout& GetMaterialTemplateParameters() const {
        return material_parameters;
    }
    
    std::shared_ptr<Material> GetDefaultMaterial() {
        return default_material;
    }
    
    const std::string& GetName() const { return material_name; }
    
protected:
    struct Private {}; //Used to ensure constructor is not called outside the CreateTemplate function, without making it private(since make_shared needs it)
    MaterialTemplate(const MaterialLayout& layout, std::string name);

protected:

    std::string material_name = "";
    std::shared_ptr<Material> default_material;
    MaterialLayout material_parameters;
    std::unordered_map<std::string, size_t> material_parameters_map;
};

class Material : public std::enable_shared_from_this<Material> {
public:
    Material() = default;
    Material(std::shared_ptr<MaterialTemplate> material_template, std::shared_ptr<MaterialProxy> material_proxy = nullptr);
    virtual ~Material() {}
    
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

    
    void SetParameter(const std::string& name, std::shared_ptr<RenderTexture2DResource> value, std::shared_ptr<TextureProxy> proxy = nullptr);
    
    void SetTexture(const std::string& name, const std::string& path);
    void SetTexture(const std::string& name, std::shared_ptr<TextureProxy> texture_proxy);

    Material_status GetStatus() const {
        return status;
    }

    const std::string& GetFilePath() const {
        if(material_proxy) {
            return material_proxy->GetFilePath();
        } else {
            return "";
        }
    }

    const std::string& GetNativeFilePath() const {
        if(material_proxy) {
            return material_proxy->GetNativeFilePath();
        } else {
            return "";
        }
    }

    std::shared_ptr<MaterialProxy> GetMaterialProxy() const {
        return material_proxy;
    }

    void SetMaterialProxy(std::shared_ptr<MaterialProxy> proxy) {
        material_proxy = proxy;
    }

    const std::vector<MaterialParameter>& GetMaterialParameters() const {
        return parameters;
    }


    std::shared_ptr<MaterialTemplate> GetMaterialTemplate() const {
        if(auto ptr = material_template.lock()) {
            return ptr;
        } else {
            throw std::runtime_error("The material template was unloaded but a material attempted to use it.\n");
        }
        
    }

    virtual void UpdateValues(std::shared_ptr<RenderCommandList> command_list) = 0;

    
protected:
    void SetParameterTypeDefault(MaterialParameter& param);
    friend class MaterialManager;
    friend class RenderCommandList;
#ifdef EDITOR
    friend class MaterialEditor;
#endif

    std::shared_ptr<MaterialProxy> material_proxy = nullptr;
    std::weak_ptr<MaterialTemplate> material_template;
    std::vector<MaterialParameter> parameters = std::vector<MaterialParameter>();
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

template<typename T>
inline void Material::SetParameter(const std::string& name, T value) {
    auto& param = parameters[GetMaterialTemplate()->GetMaterialTemplateParameterIndex(name)];
    param.flags |= MaterialParameter_flags::DIRTY;
    param.flags &= ~MaterialParameter_flags::DEFAULT;
    if (!std::holds_alternative<T>(param.resource) && !std::holds_alternative<std::monostate>(param.resource)) throw std::runtime_error("Parameter " + name + "assignment type mismatch");
    param.resource = value;
}


inline void Material::SetParameter(const std::string& name, std::shared_ptr<RenderTexture2DResource> value, std::shared_ptr<TextureProxy> proxy) {
    auto& param = parameters[GetMaterialTemplate()->GetMaterialTemplateParameterIndex(name)];
    param.flags |= MaterialParameter_flags::DIRTY;
    param.flags &= ~MaterialParameter_flags::DEFAULT;
    if (param.type != MaterialLayoutItemType::TEXTURE) throw std::runtime_error("Parameter " + name + "assignment type mismatch");
    MaterialTextureType type;
    type.texture = value;
    type.texture_proxy = proxy;
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

    std::shared_ptr<Material> CreateMaterialFromProxy(std::shared_ptr<MaterialProxy> proxy);

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
        std::shared_ptr<TextureProxy> proxy;
        bool destroyed = false;
    };
    void AddTextureLoad(std::shared_ptr<Material> material, std::string name, Future<std::shared_ptr<RenderTexture2DResource>> future, std::shared_ptr<TextureProxy> proxy);

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
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
        SCALAR = 0, VEC2 = 1, VEC3 = 2, VEC4 = 3, TEXTURE = 4, MAT3 = 5, MAT4 = 6, INVALID_PARAMETER = -1
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

    using material_parameter_type = std::variant<float, glm::vec2, glm::vec3, glm::vec4, std::shared_ptr<RenderTexture2DResource>>;
    using material_resource_type = std::variant<RenderDescriptorTable,std::shared_ptr<RenderBufferResource>>;

    enum class Material_status : char {
        OK = 0, ERROR = 1, UNINITIALIZED = 2
    };

    struct MaterialResource {
        int index = -1;
        material_resource_type resource;
        bool is_table = false;
    };

    struct MaterialParameter {
        material_parameter_type resource;
        std::string name;
        MaterialTemplate::MaterialTemplateParameterType type;
        bool dirty = false;
        bool table = false;
    };

    void SetMaterial(RenderCommandList* command_list, std::shared_ptr<Pipeline> pipeline);

    template<typename T>
    void SetParameter(const std::string& name, T value) {
        auto& param = parameters[material_template->GetMaterialTemplateParameterIndex(name)];
        param.dirty = true;
        if (!std::holds_alternative<T>(param.resource)) throw std::runtime_error("Parameter " + name + "assignment type mismatch");
        param.resource = value;
    }

    void SetTexture(const std::string& name, const std::string& path);

    Material_status GetStatus() const {
        return status;
    }

    const std::string& GetFilePath() const {
        return material_path;
    }

private:
    friend class MaterialManager;
    Material(std::shared_ptr<MaterialTemplate> material_template);
    void UpdateValues(RenderCommandList* command_list);
    std::string material_path;
    std::shared_ptr<MaterialTemplate> material_template;
    std::vector<MaterialParameter> parameters;
    std::vector<MaterialResource> resources;
    Material_status status = Material_status::OK;
};

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
    };
    void AddTextureLoad(std::shared_ptr<Material> material, std::string name, Future<std::shared_ptr<RenderTexture2DResource>> future);

    std::shared_ptr<Material> ParseMaterialFromFile(const std::string& path);
    std::shared_ptr<Material> ParseMaterialFromString(const std::string& string, std::shared_ptr<Shader> shader_spec = nullptr);

    std::mutex material_mutex;
    std::unordered_map<std::string, std::shared_ptr<MaterialTemplate>> material_templates;
    std::unordered_map<std::string, std::shared_ptr<Material>> materials;
    std::mutex material_load_mutex;
    std::deque<Material_loading_item> material_load;
    static MaterialManager* instance;
};
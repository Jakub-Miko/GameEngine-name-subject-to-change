#include "LocalPropertySetModule.h"
#include <World/World.h>
#include <World/Components/DynamicPropertiesComponent.h>
#include <World/ScriptModules/LocalEntityModule.h>
#include <Application.h>

template<typename T>
T GetProperty(std::string name) {
    auto& props = Application::GetWorld().GetComponent<DynamicPropertiesComponent>(Entity(GetCurrentEntity_L().id)).m_Properties;
    auto find = props.find(name);
    if (find != props.end()) {
        Script_Variant_type& prop = (*find).second;
        try {

            T& value = std::get<T>(prop);
            return value;

        }
        catch (std::bad_variant_access& e) {
            //std::cout << e.what() << "\n";
            throw std::runtime_error("Invalid Property Access");
        }
    }
    else {
        throw std::runtime_error("Property " + name + " was not found");
    }
}

template<typename T>
void SetProperty(std::string name, T value) {
    auto& props = Application::GetWorld().GetComponent<DynamicPropertiesComponent>(Entity(GetCurrentEntity_L().id)).m_Properties;
    auto find = props.find(name);
    if (find != props.end()) {
        Script_Variant_type& prop = (*find).second;
        if (std::get_if<T>(&prop)) {
            prop.emplace<T>(value);
            return;
        }
        else {
            throw std::runtime_error("Invalid Property Access");
        }
    }
    else {
        props.insert(std::make_pair(name, value));
        return;
    }
}

template<typename T>
void CreateProperty(std::string name, T value) {
    auto& props = Application::GetWorld().GetComponent<DynamicPropertiesComponent>(Entity(GetCurrentEntity_L().id)).m_Properties;
    auto find = props.find(name);
    if (find != props.end()) {
        Script_Variant_type& prop = (*find).second;
        if (std::get_if<T>(&prop)) {
            return;
        }
        else {
            throw std::runtime_error("Invalid Property Access");
        }
    }
    else {
        props.insert(std::make_pair(name, value));
        return;
    }
}


extern "C" {
    int GetProperty_INT_L(const char * name) {
        return GetProperty<int>(name);
    }

    float GetProperty_FLOAT_L(const char * name) {
        return GetProperty<float>(name);
    }

    const char * GetProperty_STRING_L(const char * name) {
        auto val = GetProperty<std::string>(name);
        char* copy = new char[val.size() + 1];
        std::copy(val.begin(), val.end(), copy);
        copy[val.size()] = '\0';

        return copy;
    }
    
    vec2 GetProperty_VEC2_L(const char * name) {
        auto value = GetProperty<glm::vec2>(name);
        return *reinterpret_cast<vec2*>(&value);
    }
    
    vec3 GetProperty_VEC3_L(const char * name) {
        auto value = GetProperty<glm::vec3>(name);
        return *reinterpret_cast<vec3*>(&value);
    }
    
    vec4 GetProperty_VEC4_L(const char * name) {
        auto value = GetProperty<glm::vec4>(name);
        return *reinterpret_cast<vec4*>(&value);
    }

    void SetProperty_INT_L(const char* name,int value) {
        SetProperty(name, value);
    }

    void SetProperty_FLOAT_L(const char* name, float value) {
        SetProperty(name, value);
    }

    void SetProperty_STRING_L(const char* name, const char* value) {
        SetProperty(name, std::string(value));
    }

    void SetProperty_VEC2_L(const char* name, vec2 value) {
        auto val = *reinterpret_cast<glm::vec2*>(&value);
        SetProperty(name, val);
    }

    void SetProperty_VEC3_L(const char* name, vec3 value) {
        auto val = *reinterpret_cast<glm::vec3*>(&value);
        SetProperty(name, val);
    }

    void SetProperty_VEC4_L(const char* name, vec4 value) {
        auto val = *reinterpret_cast<glm::vec3*>(&value);
        SetProperty(name, val);
    }

    void CreateProperty_INT_L(const char* name, int value) {
        CreateProperty(name, value);
    }

    void CreateProperty_FLOAT_L(const char* name, float value) {
        CreateProperty(name, value);
    }

    void CreateProperty_STRING_L(const char* name, const char* value) {
        CreateProperty(name, std::string(value));
    }

    void CreateProperty_VEC2_L(const char* name, vec2 value) {
        auto val = *reinterpret_cast<glm::vec2*>(&value);
        CreateProperty(name, val);
    }

    void CreateProperty_VEC3_L(const char* name, vec3 value) {
        auto val = *reinterpret_cast<glm::vec3*>(&value);
        CreateProperty(name, val);
    }

    void CreateProperty_VEC4_L(const char* name, vec4 value) {
        auto val = *reinterpret_cast<glm::vec3*>(&value);
        CreateProperty(name, val);
    }

    bool PropertyExists_L(const char* name)
    {
        auto& props = Application::GetWorld().GetComponent<DynamicPropertiesComponent>(Entity(GetCurrentEntity_L().id)).m_Properties;
        auto find = props.find(name);
        if (find != props.end()) {
            return true;
        };
        return false;
    }

}

void LocalPropertySetModule::OnRegisterModule(ModuleBindingProperties& props)
{
    LocalEntityModule().RegisterModule(props);

    props.Add_FFI_declarations(R"(
	int GetProperty_INT_L(const char * name);
    float GetProperty_FLOAT_L(const char * name);
    const char * GetProperty_STRING_L(const char * name);
    vec2 GetProperty_VEC2_L(const char * name);
    vec3 GetProperty_VEC3_L(const char * name);
    vec4 GetProperty_VEC4_L(const char * name);

    void SetProperty_INT_L(const char* name,int value);
    void SetProperty_FLOAT_L(const char* name, float value);
    void SetProperty_STRING_L(const char* name, const char* value);
    void SetProperty_VEC2_L(const char* name, vec2 value);
    void SetProperty_VEC3_L(const char* name, vec3 value);
    void SetProperty_VEC4_L(const char* name, vec4 value);

    void CreateProperty_INT_L(const char* name,int value);
    void CreateProperty_FLOAT_L(const char* name, float value);
    void CreateProperty_STRING_L(const char* name, const char* value);
    void CreateProperty_VEC2_L(const char* name, vec2 value);
    void CreateProperty_VEC3_L(const char* name, vec3 value);
    void CreateProperty_VEC4_L(const char* name, vec4 value);

    bool PropertyExists_L(const char* name);

	)");

    props.Add_FFI_aliases({
        {"GetProperty_INT_L","GetProperty_INT"},
        {"GetProperty_FLOAT_L","GetProperty_FLOAT"},
        {"GetProperty_STRING_L","GetProperty_STRING"},
        {"GetProperty_VEC2_L","GetProperty_VEC2"},
        {"GetProperty_VEC3_L","GetProperty_VEC3"},
        {"GetProperty_VEC4_L","GetProperty_VEC4"},

        {"SetProperty_INT_L","SetProperty_INT"},
        {"SetProperty_FLOAT_L","SetProperty_FLOAT"},
        {"SetProperty_STRING_L","SetProperty_STRING"},
        {"SetProperty_VEC2_L","SetProperty_VEC2"},
        {"SetProperty_VEC3_L","SetProperty_VEC3"},
        {"SetProperty_VEC4_L","SetProperty_VEC4"},

        {"CreateProperty_INT_L","CreateProperty_INT"},
        {"CreateProperty_FLOAT_L","CreateProperty_FLOAT"},
        {"CreateProperty_STRING_L","CreateProperty_STRING"},
        {"CreateProperty_VEC2_L","CreateProperty_VEC2"},
        {"CreateProperty_VEC3_L","CreateProperty_VEC3"},
        {"CreateProperty_VEC4_L","CreateProperty_VEC4"},

        {"PropertyExists_L","PropertyExists"}

        });

}

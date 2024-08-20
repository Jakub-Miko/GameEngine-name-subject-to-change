#include "LocalPropertySetModule.h"
#include <World/World.h>
#include <World/Components/DynamicPropertiesComponent.h>
#include <World/ScriptModules/LocalEntityModule.h>
#include <Application.h>
#include <Core/Defines.h>

/**
 * @brief Template used for retrieving the DynamicPropertiesComponent properties of the currently processed entity from lua
 * @tparam T The type of the property to get
 * @param name the name of the property to get
 * @return the property value
 * 
 * @warning this function throws a runtime_error when a property doesn't exist, or an invalid type was provided
 * 
 * @lua
 */
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

/**
 * @brief Template used for setting the DynamicPropertiesComponent properties of the currently processed entity from lua
 * @tparam T The type of the property to set
 * @param name the name of the property to set
 * @param value the property value to set 
 * 
 * @warning this function throws a runtime_error when a property exists, but an invalid type was provided
 * 
 * @lua
 */
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

/**
 * @brief Template used for creating the DynamicPropertiesComponent properties of the currently processed entity from lua
 * 
 * Unlike @ref SetProperty(std::string, T) "SetProperty" this function only sets the value if it didn't exist and doesn't replace it.
 * 
 * @tparam T The type of the property to create
 * @param name the name of the property to create
 * @param value the initial value of the property if it doesn't already exist
 * 
 * @warning this function throws a runtime_error when a property exists, but an invalid type was provided
 * 
 * @lua
 */
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

    /**
     * @brief Get an integer property from the DynamicPropertiesComponent of the currently processed entity 
     * @param name the name of the property
     * @return the value of the property 
     * @lua
     */
    LIBEXP int GetProperty_INT_L(const char * name) {
        return GetProperty<int>(name);
    }

    /**
     * @brief Get a float property from the DynamicPropertiesComponent of the currently processed entity
     * @param name the name of the property
     * @return the value of the property
     * @lua
     */
    LIBEXP float GetProperty_FLOAT_L(const char * name) {
        return GetProperty<float>(name);
    }

    /**
     * @brief Get a string property from the DynamicPropertiesComponent of the currently processed entity
     * @param name the name of the property
     * @return the value of the property
     * @lua
     */
    LIBEXP const char * GetProperty_STRING_L(const char * name) {
        auto val = GetProperty<std::string>(name);
        char* copy = new char[val.size() + 1];
        std::copy(val.begin(), val.end(), copy);
        copy[val.size()] = '\0';

        return copy;
    }
    
    /**
     * @brief Get a vec2 property from the DynamicPropertiesComponent of the currently processed entity
     * @param name the name of the property
     * @return the value of the property
     * @lua
     */
    LIBEXP vec2 GetProperty_VEC2_L(const char * name) {
        auto value = GetProperty<glm::vec2>(name);
        return *reinterpret_cast<vec2*>(&value);
    }
    
    /**
     * @brief Get a vec3 property from the DynamicPropertiesComponent of the currently processed entity
     * @param name the name of the property
     * @return the value of the property
     * @lua
     */
    LIBEXP vec3 GetProperty_VEC3_L(const char * name) {
        auto value = GetProperty<glm::vec3>(name);
        return *reinterpret_cast<vec3*>(&value);
    }
    
    /**
     * @brief Get a vec4 property from the DynamicPropertiesComponent of the currently processed entity
     * @param name the name of the property
     * @return the value of the property
     * @lua
     */
    LIBEXP vec4 GetProperty_VEC4_L(const char * name) {
        auto value = GetProperty<glm::vec4>(name);
        return *reinterpret_cast<vec4*>(&value);
    }

    /**
     * @brief Sets an integer property in the DynamicPropertiesComponent of the currently processed entity 
     * @param name the name of the property to set 
     * @param value the new value of the property
     * @lua
     */
    LIBEXP void SetProperty_INT_L(const char* name,int value) {
        SetProperty(name, value);
    }

    /**
     * @brief Sets a float property in the DynamicPropertiesComponent of the currently processed entity
     * @param name the name of the property to set
     * @param value the new value of the property
     * @lua
     */
    LIBEXP void SetProperty_FLOAT_L(const char* name, float value) {
        SetProperty(name, value);
    }

    /**
     * @brief Sets a string property in the DynamicPropertiesComponent of the currently processed entity
     * @param name the name of the property to set
     * @param value the new value of the property
     * @lua
     */
    LIBEXP void SetProperty_STRING_L(const char* name, const char* value) {
        SetProperty(name, std::string(value));
    }

    /**
     * @brief Sets a vec2 property in the DynamicPropertiesComponent of the currently processed entity
     * @param name the name of the property to set
     * @param value the new value of the property
     * @lua
     */
    LIBEXP void SetProperty_VEC2_L(const char* name, vec2 value) {
        auto val = *reinterpret_cast<glm::vec2*>(&value);
        SetProperty(name, val);
    }

    /**
     * @brief Sets a vec3 property in the DynamicPropertiesComponent of the currently processed entity
     * @param name the name of the property to set
     * @param value the new value of the property
     * @lua
     */
    LIBEXP void SetProperty_VEC3_L(const char* name, vec3 value) {
        auto val = *reinterpret_cast<glm::vec3*>(&value);
        SetProperty(name, val);
    }

    /**
     * @brief Sets a vec4 property in the DynamicPropertiesComponent of the currently processed entity
     * @param name the name of the property to set
     * @param value the new value of the property
     * @lua
     */
    LIBEXP void SetProperty_VEC4_L(const char* name, vec4 value) {
        auto val = *reinterpret_cast<glm::vec3*>(&value);
        SetProperty(name, val);
    }


    /**
     * @brief Creates and initializes an integer property in the DynamicPropertiesComponent of the currently processed entity, does nothing if it already exists
     * @param name the name of the property to set
     * @param value the new value of the property
     * @lua
     */
    LIBEXP void CreateProperty_INT_L(const char* name, int value) {
        CreateProperty(name, value);
    }

    /**
     * @brief Creates and initializes a float property in the DynamicPropertiesComponent of the currently processed entity, does nothing if it already exists
     * @param name the name of the property to set
     * @param value the new value of the property
     * @lua
     */
    LIBEXP void CreateProperty_FLOAT_L(const char* name, float value) {
        CreateProperty(name, value);
    }

    /**
     * @brief Creates and initializes a string property in the DynamicPropertiesComponent of the currently processed entity, does nothing if it already exists
     * @param name the name of the property to set
     * @param value the new value of the property
     * @lua
     */
    LIBEXP void CreateProperty_STRING_L(const char* name, const char* value) {
        CreateProperty(name, std::string(value));
    }

    /**
     * @brief Creates and initializes a vec2 property in the DynamicPropertiesComponent of the currently processed entity, does nothing if it already exists
     * @param name the name of the property to set
     * @param value the new value of the property
     * @lua
     */
    LIBEXP void CreateProperty_VEC2_L(const char* name, vec2 value) {
        auto val = *reinterpret_cast<glm::vec2*>(&value);
        CreateProperty(name, val);
    }

    /**
     * @brief Creates and initializes a vec3 property in the DynamicPropertiesComponent of the currently processed entity, does nothing if it already exists
     * @param name the name of the property to set
     * @param value the new value of the property
     * @lua
     */
    LIBEXP void CreateProperty_VEC3_L(const char* name, vec3 value) {
        auto val = *reinterpret_cast<glm::vec3*>(&value);
        CreateProperty(name, val);
    }

    /**
     * @brief Creates and initializes a vec4 property in the DynamicPropertiesComponent of the currently processed entity, does nothing if it already exists
     * @param name the name of the property to set
     * @param value the new value of the property
     * @lua
     */
    LIBEXP void CreateProperty_VEC4_L(const char* name, vec4 value) {
        auto val = *reinterpret_cast<glm::vec3*>(&value);
        CreateProperty(name, val);
    }

    /**
     * @brief Checks if a property exists in the DynamicPropertiesComponent of the currently processed entity 
     * @param name the name of the property
     * @return whether the property exists
     */
    LIBEXP bool PropertyExists_L(const char* name)
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

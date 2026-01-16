#pragma once
#include <memory>
#include <unordered_map>
#include <functional>

#include "DynamicProperties.h"
#include "DynamicPropertyModuleAdapterBase.h"
#include "DynamicPropertyStore.h"
#include "TypeList.h"

template<typename BaseClass, typename ... Types>
class DynamicPropertyProjector {
public:
    DynamicPropertyProjector() {
        Initialize_internal<Types...>();
    }

    std::vector<std::shared_ptr<BaseClass>> Project(std::shared_ptr<DynamicPropertyStore> properties) {
        std::vector<std::shared_ptr<BaseClass>> result;
        result.reserve(properties->GetProperties().size());
        for(auto& prop : properties->GetProperties()) {
            auto fnd = mapping_functions.find(prop->GetValue().type().hash_code());
            if(fnd != mapping_functions.end()) {
                result.push_back(fnd->second(prop));
            }
        }
        return std::move(result);
    }

private:

    template<typename T, typename ... Args>
    void Initialize_internal() {
        using Type = typename BaseClass::template Implementation<T>;
        static_assert(std::is_base_of_v<PropertyModuleBase, Type>);
        if(Type().IsImplemented()) {
            mapping_functions[typeid(T).hash_code()] = [](std::shared_ptr<DynamicPropertyBase> prop) {
                return std::static_pointer_cast<BaseClass>(
                    std::make_shared<Type>(
                        std::dynamic_pointer_cast<DynamicProperty<T>>(prop)));
            };
        }
        if constexpr (sizeof...(Args) > 0) {
            Initialize_internal<Args...>();
        }
    };

    using function_type = std::function<std::shared_ptr<BaseClass>(std::shared_ptr<DynamicPropertyBase>)>;
    std::unordered_map<size_t, function_type> mapping_functions;

};

template<typename BaseClass, typename... Types>
DynamicPropertyProjector<BaseClass, Types...> MakeProjector(TypeList<Types...> list) {
    return DynamicPropertyProjector<BaseClass, Types...>();
}

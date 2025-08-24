#pragma once
#include <memory>
#include <stdexcept> 

template<typename T>
class ResettableSharedFromThis {
public:

    template<typename U>
    std::shared_ptr<T> ResetSharedFromThis(U deletion_function) {
        if(shared_this.lock()) {
            throw std::runtime_error("Cannot Reset ResettableSharedFromThis when the original shared_ptr is still valid.\n");
        }

        auto new_this = std::shared_ptr<T>(static_cast<T*>(this), deletion_function);

        shared_this = new_this;

        return new_this;
    }

    template<typename U>
    std::shared_ptr<T> ResetSharedFromThis() {
        if(shared_this.lock()) {
            throw std::runtime_error("Cannot Reset ResettableSharedFromThis when the original shared_ptr is still valid.\n");
        }

        auto new_this = std::shared_ptr<T>(static_cast<T*>(this));

        shared_this = new_this;

        return new_this;
    }

    std::shared_ptr<T> SharedFromThis() {
        if(auto ptr = shared_this.lock()) {
            return ptr;
        } else {
            throw std::runtime_error("SharedFromThis called while it is not being held by a valid shared_ptr.\n");
        }
    }

public:
    std::weak_ptr<T> shared_this;
};
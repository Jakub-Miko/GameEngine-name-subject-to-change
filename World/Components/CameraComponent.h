#pragma once
#include <glm/glm.hpp>
#include <Window.h>
#include <Application.h>
#include <glm/gtc/matrix_transform.hpp>
#include <Core/Geometry.h>
#include <Core/RuntimeTag.h>

struct Frustum {
    Frustum(Plane near, Plane far, Plane right, Plane left, Plane top, Plane bottom) : near(near), far(far),
        right(right), left(left), top(top), bottom(bottom) {}

    Frustum() = default;

    Plane near, far, right, left, top, bottom;
};


class CameraComponent {
    RUNTIME_TAG("CameraComponent")

public:
    CameraComponent() : fov(45.0f), zNear(0.1), zFar(1000), view_frustum() {
        auto props = Application::Get()->GetWindow()->GetProperties();
        aspect_ratio = (float)props.resolution_x / (float)props.resolution_y;
        projection_matrix = glm::perspective(glm::radians(fov), aspect_ratio, zNear, zFar);
    }

    CameraComponent(float fov, float zNear, float zFar, float aspect_ratio) : fov(fov), zNear(zNear), zFar(zFar),
                                                                              aspect_ratio(aspect_ratio),
                                                                              view_frustum(),
                                                                              projection_matrix(
                                                                                  glm::perspective(
                                                                                      glm::radians(fov), aspect_ratio,
                                                                                      zNear, zFar)) {}

    CameraComponent(const CameraComponent& other) : fov(other.fov), zNear(other.zNear), zFar(other.zFar),
                                                    aspect_ratio(other.aspect_ratio), view_frustum(), exposure(other.exposure),
                                                    projection_matrix(
                                                        glm::perspective(
                                                            glm::radians(fov), aspect_ratio, zNear, zFar)) {}

    void UpdateViewFrustum(const glm::mat4& model_matrix) {
        glm::vec3 up = model_matrix * glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
        glm::vec3 right = model_matrix * glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);
        glm::vec3 forward = model_matrix * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f);
        glm::vec3 translation = model_matrix[3];

        float vertical_size = (glm::tan(glm::radians(fov) / 2) * zFar);
        float horizontal_size = vertical_size * aspect_ratio;

        view_frustum.near = Plane(forward, translation + zNear * forward);
        view_frustum.far = Plane(-forward, translation + zFar * forward);
        view_frustum.right = Plane(glm::normalize(glm::cross(up, (forward * zFar) + right * horizontal_size)),
                                   translation);
        view_frustum.left = Plane(glm::normalize(glm::cross((forward * zFar) - right * horizontal_size, up)),
                                  translation);
        view_frustum.top = Plane(glm::normalize(glm::cross(right, (forward * zFar) - up * vertical_size)), translation);
        view_frustum.bottom = Plane(glm::normalize(glm::cross((forward * zFar) + up * vertical_size, right)),
                                    translation);
    }

    void UpdateProjectionMatrix() {
        projection_matrix = glm::perspective(glm::radians(fov), aspect_ratio, zNear, zFar);
    }


    float GetFOV() const {
        return fov;
    }

    float GetAspectRatio() const {
        return aspect_ratio;
    }

    float GetZNear() const {
        return zNear;
    }

    float GetZFar() const {
        return zFar;
    }

    const glm::mat4& GetProjectionMatrix() const {
        return projection_matrix;
    }

    const Frustum& GetViewFrustum() const {
        return view_frustum;
    }

public:
#ifdef EDITOR
    friend class PropertiesPanel;
#endif // EDITOR

    float fov;
    float zNear;
    float zFar;
    float aspect_ratio;
    float exposure = 1.0f;

    Frustum view_frustum = Frustum();

    glm::mat4 projection_matrix = glm::mat4(1.0f);
};

inline void to_json(nlohmann::json& nlohmann_json_j, const CameraComponent& nlohmann_json_t) {
    nlohmann_json_j["fov"] = nlohmann_json_t.fov;
    nlohmann_json_j["zNear"] = nlohmann_json_t.zNear;
    nlohmann_json_j["zFar"] = nlohmann_json_t.zFar;
    nlohmann_json_j["aspect_ratio"] = nlohmann_json_t.aspect_ratio;
    nlohmann_json_j["exposure"] = nlohmann_json_t.exposure;
}

inline void from_json(const nlohmann::json& nlohmann_json_j, CameraComponent& nlohmann_json_t) {
    nlohmann_json_j.at("fov").get_to(nlohmann_json_t.fov);
    nlohmann_json_j.at("zNear").get_to(nlohmann_json_t.zNear);
    nlohmann_json_j.at("zFar").get_to(nlohmann_json_t.zFar);
    nlohmann_json_j.at("aspect_ratio").get_to(nlohmann_json_t.aspect_ratio);
    if(nlohmann_json_j.contains("exposure")) {
        nlohmann_json_j.at("exposure").get_to(nlohmann_json_t.exposure);
    }
}

#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <Core/Geometry.h>
#include <Core/RuntimeTag.h>

struct Frustum {
	Frustum(Plane near, Plane far, Plane right, Plane left, Plane top, Plane bottom) : near(near), far(far), right(right), left(left), top(top), bottom(bottom) {}
	Frustum() = default;

	Plane near, far, right, left, top, bottom;
};



class CameraComponent {
	RuntimeTag("CameraComponent")
public:
	CameraComponent(float fov, float zNear, float zFar, float aspect_ratio) : fov(fov), zNear(zNear), zFar(zFar), aspect_ratio(aspect_ratio), view_frustum(),
		projection_matrix(glm::perspective(glm::radians(fov), aspect_ratio, zNear, zFar)) 
	{
		
	}

	void UpdateViewFrustum(const glm::mat4& model_matrix) {
		glm::vec3 up = model_matrix * glm::vec4(0.0f, 1.0f, 0.0f,0.0f);
		glm::vec3 right = model_matrix * glm::vec4(1.0f, 0.0f, 0.0f,0.0f);
		glm::vec3 forward = model_matrix * glm::vec4(0.0f, 0.0f, -1.0f,0.0f);
		glm::vec3 translation = model_matrix[3];

		float vertical_size = (glm::tan(glm::radians(fov) / 2) * zFar);
		float horizontal_size = vertical_size * aspect_ratio;

		view_frustum.near = Plane(forward, translation + zNear * forward);
		view_frustum.far = Plane(-forward, translation + zFar * forward);
		view_frustum.right = Plane(glm::normalize(glm::cross(up, (forward * zFar) + right*horizontal_size)), translation);
		view_frustum.left = Plane(glm::normalize(glm::cross((forward * zFar) - right * horizontal_size,up)), translation);
		view_frustum.top = Plane(glm::normalize(glm::cross(right,(forward * zFar) - up*vertical_size)), translation);
		view_frustum.bottom = Plane(glm::normalize(glm::cross((forward * zFar) + up * vertical_size, right)), translation);
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

private:
#ifdef EDITOR
	friend class PropertiesPanel;
#endif // EDITOR

	float fov;
	float zNear;
	float zFar;
	float aspect_ratio;

	Frustum view_frustum;

	glm::mat4 projection_matrix;
};
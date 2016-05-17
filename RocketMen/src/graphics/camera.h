
#pragma once

#include <includes.h>

namespace graphics {
	enum class ProjectionMode
	{
		ORTOGRAPHIC_PROJECTION,
		PERSPECTIVE_PROJECTION
	};
}; //namespace graphics

class Camera
{

public:
	Camera(graphics::ProjectionMode projection, float width, float height);
	~Camera();

	void translate(glm::vec3 translation);
	void rotate(const glm::quat& rotation);
	void rotate(const glm::vec3& rotation);

	void setPosition(const glm::vec3& pos);
	void setRotation(const glm::quat& rot);
	void setEulerAngles(const glm::vec3& eulerAngles);
	void setViewport(int32_t x, int32_t, int32_t width, int32_t height);

	float     getFocalDepth()       const;
	glm::vec3 getPosition()         const;
	glm::quat getRotation()         const;
	glm::vec3 getEulerAngles()      const;
	glm::vec4 getViewport()         const;
	glm::mat4 getViewMatrix();
	glm::mat4 getProjectionMatrix() const;

	void updateViewMatrix();
private:

	glm::mat4 m_viewMatrix;
	glm::vec4 m_viewport;
	glm::mat4 m_projectionMatrix;
	glm::vec3 m_position;
	glm::quat m_rotation;

	bool  m_isDirty;
	float m_focalDepth;

public:
	static Camera* mainCamera;
};
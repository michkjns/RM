
#pragma once

#include <common.h>

#include <graphics/renderer.h>

namespace graphics {
	enum class ProjectionMode
	{
		ORTOGRAPHIC_PROJECTION,
		PERSPECTIVE_PROJECTION
	};
}; //namespace graphics

class Camera // TODO(Rename Camera2D?)
{
public:
	Camera(graphics::ProjectionMode projection, float width, float height,
		   int32_t pixelsPerMeter);
	~Camera();

	void translate(Vector3 translation);
	void rotate(const glm::quat& rotation);
	void rotate(const Vector3& rotation);
	void scale(const Vector3& scale);

	void setPosition(const Vector3& pos);
	void setRotation(const glm::quat& rot);
	void setEulerAngles(const Vector3& eulerAngles);
	void setScale(const Vector3& scale);

	int32_t   getPixelsPerMeter()   const;
	Vector3   getPosition()         const;
	glm::quat getRotation()         const;
	Vector3   getEulerAngles()      const;
	glm::mat4 getViewMatrix();
	glm::mat4 getProjectionMatrix() const;

	Vector2 screenToWorld(const Vector2& screenPoint);

	void updateViewMatrix();
private:

	glm::mat4 m_viewMatrix;
	glm::mat4 m_projectionMatrix;
	Vector3   m_position;
	Vector3   m_scale;
	glm::quat m_rotation;
	Vector2   m_projSize;

	bool    m_isDirty;
	int32_t m_pixelsPerMeter;

public:
	static Camera* mainCamera;
};

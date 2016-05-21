
#pragma once

#include <includes.h>

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

	Vector2 screenToWorld(const Vector2& screenPoint)
	{
/*
		Vector2 normalized = Vector2( -1.0f + 2.0f * point.x / view.x,
		                               1.0f - 2.0f * point.y / view.y);

		glm::mat4 invMat = getViewMatrix();
		Vector2 result = Vector2(invMat[0][0] * normalized.x + invMat[1][0] * normalized.y + invMat[3][0],
		                         invMat[0][1] * normalized.x + invMat[1][1] * normalized.y + invMat[3][1]);
*/
		const Vector2 ratio    = Vector2(1.f / (m_pixelsPerMeter * m_scale.x), 
										 -1.f / (m_pixelsPerMeter * m_scale.y));
		const Vector2 view     = Renderer::get()->getScreenSize();
		const Vector2 worldPos = Vector2(screenPoint.x * ratio.x - (view.x * ratio.x / 2.f),
		                                 screenPoint.y * ratio.y - (view.y * ratio.y / 2.f))
		                         + Vector2(m_position.x, m_position.y );
		return worldPos;
	}

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
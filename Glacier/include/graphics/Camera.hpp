#pragma once

#include "core/common.hpp"

#include <glm/glm.hpp>

namespace glacier
{
	class Camera
	{
	protected:
		GLACIER_API Camera(const glm::vec3& position, float pitch, float yaw);
		
		glm::vec3 m_Position;
		glm::vec3 m_Direction;
		float m_Pitch;
		float m_Yaw;
	public:
		GLACIER_API virtual glm::mat4 getViewMatrix() const;
		virtual glm::mat4 getProjMatrix() const = 0;
		
		GLACIER_API virtual void rotate(float pitch, float yaw);
		GLACIER_API virtual void moveLocal(const glm::vec3& vector);
	};

	class OrthographicCamera : public Camera
	{
	public:
		GLACIER_API OrthographicCamera(const glm::vec3& position, float pitch, float yaw, float aspectRatio);

		GLACIER_API void setAspectRatio(float aspectRatio);

		GLACIER_API glm::mat4 getProjMatrix() const override;
	private:
		float m_AspectRatio;
	};

	class PerspectiveCamera : public Camera
	{
	public:
		GLACIER_API PerspectiveCamera(const glm::vec3& position, float pitch, float yaw, float aspectRatio, float fov);

		GLACIER_API void setAspectRatio(float aspectRatio);
		GLACIER_API void setFOV(float fov);
		
		GLACIER_API glm::mat4 getProjMatrix() const override;
	private:
		float m_AspectRatio;
		float m_FOV;
	};
}

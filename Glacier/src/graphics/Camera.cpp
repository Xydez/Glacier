#include "graphics/Camera.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <cmath>

const glm::vec3 UP(0.0f, 1.0f, 0.0f);

glm::vec3 vectorFromEuler(float pitch, float yaw)
{
	return glm::normalize(glm::vec3(cos(yaw) * cos(pitch), sin(pitch), sin(yaw) * cos(pitch)));
}

glacier::Camera::Camera(const glm::vec3& position, float pitch, float yaw)
	: m_Position(position), m_Pitch(pitch), m_Yaw(yaw), m_Direction(vectorFromEuler(pitch, yaw))
{
}

glm::mat4 glacier::Camera::getViewMatrix() const
{
	return glm::lookAt(m_Position, m_Position + m_Direction, UP);
}

void glacier::Camera::rotate(float pitch, float yaw)
{
	m_Pitch += pitch;
	m_Yaw += yaw;

	m_Direction = vectorFromEuler(m_Pitch, m_Yaw);
}

void glacier::Camera::moveLocal(const glm::vec3& vector)
{
	m_Position += vector.x * glm::normalize(glm::cross(m_Direction, UP));
	m_Position.y += vector.y;
	m_Position += vector.z * m_Direction;
}

glacier::OrthographicCamera::OrthographicCamera(const glm::vec3& position, float pitch, float yaw, float aspectRatio)
	: Camera(position, pitch, yaw), m_AspectRatio(aspectRatio)
{
}

void glacier::OrthographicCamera::setAspectRatio(float aspectRatio)
{
	m_AspectRatio = aspectRatio;
}

glm::mat4 glacier::OrthographicCamera::getProjMatrix() const
{
	return glm::ortho(-m_AspectRatio, m_AspectRatio, -1.0f, 1.0f, 0.1f, 100.0f);
}

glacier::PerspectiveCamera::PerspectiveCamera(const glm::vec3& position, float pitch, float yaw, float aspectRatio, float fov)
	: Camera(position, pitch, yaw), m_AspectRatio(aspectRatio), m_FOV(fov)
{
}

void glacier::PerspectiveCamera::setAspectRatio(float aspectRatio)
{
	m_AspectRatio = aspectRatio;
}

void glacier::PerspectiveCamera::setFOV(float fov)
{
	m_FOV = fov;
}

glm::mat4 glacier::PerspectiveCamera::getProjMatrix() const
{
	return glm::perspective(m_FOV, m_AspectRatio, 0.1f, 100.0f);
}

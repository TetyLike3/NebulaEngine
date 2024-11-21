#include "../VulkanRenderer.h"

#include "Camera.h"

Camera::Camera() {
	m_pUtilities = Utilities::getInstance();

	m_cameraPosition = glm::vec3(0.0f, 0.0f, -10.0f);
	m_cameraDirection = glm::vec3(0.0f, 0.0f, 0.0f);
	m_cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
	m_cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

	m_yaw = 0.0f;
	m_pitch = 0.0f;

	// Set default settings
	m_cameraSpeed = 1.0f; 
	m_cameraSensitivity = 1.0f;
}

void Camera::move(glm::vec3 dirInput, glm::vec3 angInput) {
	// Normalize inputs
	if (dirInput != glm::vec3(0.0f)) dirInput = glm::normalize(dirInput);
	if (angInput != glm::vec3(0.0f)) angInput = glm::normalize(angInput);

	// Move camera
	if (abs(dirInput.z) > 0.0f) m_cameraPosition += (dirInput.z * m_cameraSpeed) * m_cameraFront;
	if (abs(dirInput.y) > 0.0f) m_cameraPosition += m_cameraUp * (dirInput.y * m_cameraSpeed);
	if (abs(dirInput.x) > 0.0f) m_cameraPosition += glm::normalize(glm::cross(m_cameraFront, m_cameraUp)) * (dirInput.x * m_cameraSpeed);

	// Rotate camera
	m_yaw += angInput.y * m_cameraSensitivity;
	m_pitch += angInput.x * m_cameraSensitivity;

	m_cameraDirection.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
	m_cameraDirection.y = sin(glm::radians(m_pitch));
	m_cameraDirection.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
	m_cameraFront = glm::normalize(m_cameraDirection);

	// Clamp camera x rotation
	if (m_pitch > 80.0f) m_pitch = 80.0f;
	if (m_pitch < -80.0f) m_pitch = -80.0f;
}
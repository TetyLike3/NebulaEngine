#pragma once

#include "../Utilities/Utilities.h"
#include "../Graphics/Window.h"

class Camera
{
public:
	Camera();

	void move(glm::vec3 dirInput, glm::vec3 angInput);

private:
	Utilities* m_pUtilities = nullptr;
	friend class VulkanEngine;
	friend class Window;

	glm::vec3 m_cameraPosition;
	glm::vec3 m_cameraDirection;
	glm::vec3 m_cameraFront;
	glm::vec3 m_cameraUp;

	float m_yaw;
	float m_pitch;

	float m_cameraSpeed;
	float m_cameraSensitivity;
};
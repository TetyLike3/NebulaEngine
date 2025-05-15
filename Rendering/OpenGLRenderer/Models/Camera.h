#pragma once

#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera
{
public:
	Camera();

	void move(glm::vec3 dirInput, glm::vec3 angInput);
	glm::mat4 getViewMatrix();
	void setCameraSpeed(float newSpeed) { m_cameraSpeed = newSpeed; };
	void setCameraSensitivity(float newSensitivity) { m_cameraSensitivity = newSensitivity; };

private:
	friend class Window;

	glm::vec3 m_cameraPosition;
	glm::vec3 m_cameraTarget;
	glm::vec3 m_cameraDirection;
	glm::vec3 m_cameraFront;
	glm::vec3 m_cameraRight;
	glm::vec3 m_cameraUp;

	float m_yaw;
	float m_pitch;

	float m_cameraSpeed;
	float m_cameraSensitivity;
};
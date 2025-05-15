#pragma once

#include <iostream>

#include <GLAD/glad.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../Settings.h"

class Camera;

class Window
{
public:
	Window();

	int init(Settings::Window windowSettings);

	void updateLoop();

	void cleanup();

	GLFWwindow* getWindow() { return m_pWindow; }
	Camera* getCurrentCamera() { return m_pCurrentCamera; }
	void setCurrentCamera(Camera* pNewCamera) { m_pCurrentCamera = pNewCamera; }

	bool m_framebufferResized = false;
private:
	Camera* m_pCurrentCamera = nullptr;

	GLFWwindow* m_pWindow = nullptr;

	void drawFrame();
};

#pragma once

#include <iostream>
#include <map>
#include <thread>
#include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Utilities.h"
#include "Settings.h"
#include "Graphics/Window.h"
#include "Graphics/BaseShader.h"
#include "Models/BaseModel.h"
#include "Models/Camera.h"

enum class OpenGLRendererState
{
	NONE,
	INIT,
	RUNNING,
	CLEANUP,
	EXIT
};

struct sSettings {
	struct sControlSettings {
		float cameraSensitivity = .1f; // Sensitivity of the camera movement.
		float cameraSpeed = 0.05f; // Speed of the camera movement.
	} control;
};

struct sVertexData {
	glm::vec3 pos;
	glm::vec3 color;
	glm::vec2 texCoord;
};

class OpenGLRenderer
{
public:
	static OpenGLRenderer* getInstance();
	static void destroyInstance();
	OpenGLRendererState getState() { return m_state; }
	Window* getWindow() { return m_pWindow; }
	Camera* getCamera() { return m_pCamera; }
	int init(Settings* initSettings);
private:
	OpenGLRenderer();
	void loadShaders();
	void mainLoop();
	void updateUniformBuffers();
	void cleanup();

	static OpenGLRenderer* m_pInstance;
	OpenGLRendererState m_state = OpenGLRendererState::NONE;
	Settings m_currentSettings;
	Window* m_pWindow = nullptr;
	Camera* m_pCamera = nullptr;
	bool m_shouldRender = true;
	double m_startRenderTime = 0.0f;
	double m_elapsedRenderTime = 0.0f;
	double m_deltaRenderTime = 0.0f;
	unsigned int m_defaultUBO = 0;
};

inline void nativeDebugPrint(std::string message, bool newLine = false) { std::cout << (newLine ? "\n" : "") << "OpenGLRenderer DEBUG - " << message << std::endl; }
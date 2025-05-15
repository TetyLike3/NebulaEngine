#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <chrono>

#include "../Utilities/Utilities.h"
#include "Swapchain.h"



//const uint32_t WIDTH = 1280;
//const uint32_t HEIGHT = 720;

class CommandBuffer;
class UniformBufferObject;
class Camera;

class Window
{
public:
	Window();

	void initWindow();
	void createSurface();
	void createSyncObjects();

	void mainLoop();

	void cleanupSyncObjects();
	void cleanupSurface();
	void cleanupWindow();

	static void framebufferResizeCallback(GLFWwindow* window, int width, int height);

	GLFWwindow* getWindow() { return m_pWindow; }
	VkSurfaceKHR* getSurface() { return &m_surface; }
	Camera* getCamera() { return m_pCamera; }
	void setCamera(Camera* pCamera) { m_pCamera = pCamera; }
	void setVBOCount(size_t count) { m_vboCount = count; }

	bool m_framebufferResized = false;
private:
	VkInstance* m_pVkInstance = nullptr;
	Utilities* m_pUtilities = nullptr;
	VkDevice* m_pLogicalDevice = nullptr;
	VkQueue* m_pGraphicsQueue = nullptr;
	Swapchain* m_pSwapchain = nullptr;
	CommandBuffer* m_pCommandBuffer = nullptr;
	UniformBufferObject* m_pUniformBufferObject = nullptr;
	sSettings::sGraphicsSettings* m_pGraphicsSettings = nullptr;
	Camera* m_pCamera = nullptr;

	GLFWwindow* m_pWindow = nullptr;
	VkSurfaceKHR m_surface = nullptr;
	std::vector<VkSemaphore> m_imageAvailableSemaphores = {};
	std::vector<VkSemaphore> m_renderFinishedSemaphores = {};
	std::vector<VkFence> m_inFlightFences = {};
	int m_MAX_FRAMES_IN_FLIGHT = 0;
	uint32_t m_currentFrame = 0;
	bool* m_pShouldRender = nullptr;

	// Debug information
	double m_lastTime = 0.0f;
	int m_frameCounter = 0;
	double m_cpuWorkTime = 0.0f;
	double m_gpuDrawTime = 0.0f;
	double m_renderTargetDelta = 0.0f;
	double m_renderLastTime = 0.0f;
	size_t m_vboCount = 0;


	void drawFrame();
	void updateUniformBuffers(uint32_t currentImage);

	// Calculates and prints the FPS
	void calculateFPS();
};


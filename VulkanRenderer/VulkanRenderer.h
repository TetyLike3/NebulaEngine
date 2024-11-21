#pragma once

#include <iostream>
#include <string>
#include <map>
#include <vector>


#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>


#include "Utilities/Utilities.h"
#include "Utilities/DebugMessenger.h"
#include "Graphics/Window.h"
#include "Graphics/Devices.h"
#include "Graphics/Swapchain.h"
#include "Graphics/GraphicsPipeline.h"
#include "Graphics/Buffers.h"
#include "Graphics/Image.h"
#include "Models/Model.h"
#include "Models/Camera.h"


enum class VkEngineState
{
	NONE,
	INIT,
	RUNNING,
	CLEANUP,
	EXIT
};


class VulkanEngine
{
public:
	static VulkanEngine* getInstance();
	static void destroyInstance();

	VkEngineState getState() { return m_state; }
	Window* getWindow() { return m_pWindow; }
	Camera* getCamera() { return m_pCamera; }
	bool* getShouldRender() { return &m_shouldRender; }

	void run(std::map<std::string,uint32_t> versions, sSettings* settings);

private:
	VulkanEngine();

	friend class BufferManager;
	friend class PhysicalDevice;
	friend class LogicalDevice;
	friend class GraphicsPipeline;
	friend class Swapchain;
	friend class Window;

	friend void enableInputProcessing(VulkanEngine* pVulkanEngine);

	static VulkanEngine* m_pInstance;
	VkEngineState m_state = VkEngineState::NONE;

	Utilities* m_pUtilities = nullptr;

	static DebugMessenger* m_pDebugMessenger;

	std::vector<Model*> m_LoadedModels;
	Image* m_pTextureImage = nullptr;
	glm::vec3 m_sunDirection = glm::vec3(0.0f, -1.0f, 0.0f);

	VkInstance m_vkInstance = VK_NULL_HANDLE;
	Window* m_pWindow = nullptr;
	VkSurfaceKHR* m_pVkSurface = nullptr;
	PhysicalDevice* m_pPhysicalDevice = nullptr;
	VkPhysicalDevice* m_pVkPhysicalDevice = nullptr;
	LogicalDevice* m_pLogicalDevice = nullptr;
	VkDevice* m_pVkDevice = nullptr;
	Swapchain* m_pSwapchain = nullptr;
	GraphicsPipeline* m_pGraphicsPipeline = nullptr;
	BufferManager* m_pBufferManager = nullptr;
	Camera* m_pCamera = nullptr;

	bool m_shouldRender = false;
	int m_MAX_FRAMES_IN_FLIGHT = 1;
	sSettings* m_settings = nullptr;

	std::map<std::string, uint32_t> m_versions = {};


	void initVulkan();
	void createInstance();
	void mainLoop();
	void cleanup();

	bool checkValidationLayerSupport();
	std::vector<const char*> getRequiredExtensions();
	void validateSettings();

	void rebuildGraphicsPipeline();
};

inline void nativeDebugPrint(std::string message, bool newLine = false) { std::cout << (newLine ? "\n" : "") << "VulkanEngine NATIVEDEBUG - " << message << std::endl; }
#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

#include <set>
#include <vector>
#include <stdexcept>
#include <optional>

#include "QueueFamilyIndices.h"



const std::vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};


struct SwapChainSupportDetails {
	VkSurfaceCapabilitiesKHR capabilities = {};
	std::vector<VkSurfaceFormatKHR> formats = {};
	std::vector<VkPresentModeKHR> presentModes = {};
};






class PhysicalDevice
{
public:
	PhysicalDevice();

	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice physicalDevice);

	VkPhysicalDevice* getVkPhysicalDevice() { return &m_physicalDevice; };

private:
	VkInstance* m_pVkInstance = nullptr;
	VkSurfaceKHR* m_pSurface = nullptr;

	Utilities* m_pUtilities = nullptr;

	VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;


	VkPhysicalDevice* pickPhysicalDevice();

	bool isDeviceSuitable(VkPhysicalDevice candidateDevice);
	bool checkDeviceExtensionSupport(VkPhysicalDevice candidateDevice);
};


class LogicalDevice
{
public:
	LogicalDevice();


	void cleanup();

	VkDevice* getVkDevice() { return &m_logicalDevice; };
	PhysicalDevice* getPhysicalDevice() { return m_pPhysicalDevice; };
	VkQueue* getGraphicsQueue() { return &m_graphicsQueue; };

private:
	PhysicalDevice* m_pPhysicalDevice = nullptr;
	VkSurfaceKHR* m_pSurface = nullptr;
	GLFWwindow* m_pWindow = nullptr;

	Utilities* m_pUtilities = nullptr;

	VkDevice m_logicalDevice = VK_NULL_HANDLE;

	VkQueue m_graphicsQueue = VK_NULL_HANDLE;
	VkQueue m_presentQueue = VK_NULL_HANDLE;


	void createLogicalDevice();
};
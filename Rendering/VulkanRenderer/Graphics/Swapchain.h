#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

class BufferManager;
class PhysicalDevice;

class Swapchain
{
public:
	Swapchain();

	void cleanup();

	void createSwapchain();
	void createImageViews();
	void recreateSwapchain(GLFWwindow* pWindow);

	VkSwapchainKHR* getSwapchain() { return &m_swapchain; }
	VkExtent2D* getSwapchainExtent() { return &m_swapchainExtent; }
	VkFormat* getSwapchainImageFormat() { return &m_swapchainImageFormat; }
	std::vector<VkImageView>* getSwapchainImageViews() { return &m_swapchainImageViews; }

private:
	Utilities* m_pUtilities = nullptr;
	VkDevice* m_pLogicalDevice = nullptr;
	PhysicalDevice* m_pPhysicalDevice = nullptr;
	GLFWwindow* m_pWindow = nullptr;
	VkSurfaceKHR* m_pSurface = nullptr;
	//VkRenderPass* m_pRenderPass = nullptr;
	BufferManager* m_pBufferManager = nullptr;

	VkSwapchainKHR m_swapchain = nullptr;
	std::vector<VkImage> m_swapchainImages = {};
	VkFormat m_swapchainImageFormat = VK_FORMAT_UNDEFINED;
	VkExtent2D m_swapchainExtent = {};
	std::vector<VkImageView> m_swapchainImageViews = {};

	bool m_firstRun = true;

	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
};


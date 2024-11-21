#include "../VulkanRenderer.h"
#include "Window.h"
#include "Image.h"

#include "Swapchain.h"

//#define mDebugPrint(...) if(m_firstRun) {m_pUtilities->debugPrint(...,this)}


// TODO: Disable debug prints after first swapchain creation


Swapchain::Swapchain() : m_pLogicalDevice(VulkanEngine::getInstance()->m_pLogicalDevice->getVkDevice()), m_pPhysicalDevice(VulkanEngine::getInstance()->m_pPhysicalDevice), m_pWindow(VulkanEngine::getInstance()->m_pWindow->getWindow()),
	m_pSurface(VulkanEngine::getInstance()->m_pVkSurface), m_pBufferManager(VulkanEngine::getInstance()->m_pBufferManager), m_pUtilities(Utilities::getInstance())
{
	createSwapchain();
	createImageViews();
};

// TODO: Add ranking system to choose best swap chain format
VkSurfaceFormatKHR Swapchain::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
	//mDebugPrint("Choosing swap surface format...");

	for (const auto& availableFormat : availableFormats)
	{
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
			availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			//mDebugPrint("Swap surface format chosen!");
			return availableFormat;
		}
	}

	//mDebugPrint("No optimal choices, choosing first result...");
	return availableFormats[0];
}

VkPresentModeKHR Swapchain::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
	//mDebugPrint("Choosing swap present mode...");

	for (const auto& availablePresentMode : availablePresentModes)
	{
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			//mDebugPrint("Swap present mode chosen!");
			return availablePresentMode;
		}
	}

	//mDebugPrint("No optimal choices, choosing first result...");
	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D Swapchain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
{
	//mDebugPrint("Choosing swap extent...");

	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
		return capabilities.currentExtent;
	}
	else {
		int width, height;
		glfwGetFramebufferSize(m_pWindow, &width, &height);

		VkExtent2D actualExtent = {
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)
		};
		
		actualExtent.width = glm::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		actualExtent.height = glm::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

		return actualExtent;
	}
}

void Swapchain::createSwapchain()
{
	//mDebugPrint("Creating swap chain...");
	SwapChainSupportDetails swapChainSupport = m_pPhysicalDevice->querySwapChainSupport(*m_pPhysicalDevice->getVkPhysicalDevice());

	VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
	VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
	VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
	{
		imageCount = swapChainSupport.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR createInfo{
		.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		.surface = *m_pSurface,
		.minImageCount = imageCount,
		.imageFormat = surfaceFormat.format,
		.imageColorSpace = surfaceFormat.colorSpace,
		.imageExtent = extent,
		.imageArrayLayers = 1,
		.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
	};

	QueueFamilyIndices::sQueueFamilyIndices indices = QueueFamilyIndices::findQueueFamilies(*m_pPhysicalDevice->getVkPhysicalDevice(), *m_pSurface);
	uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

	if (indices.graphicsFamily != indices.presentFamily)
	{
		//mDebugPrint("Graphics and present families are different, using concurrent sharing mode...");
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else
	{
		//mDebugPrint("Graphics and present families are the same, using exclusive sharing mode...");
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0; // Optional
		createInfo.pQueueFamilyIndices = nullptr; // Optional
	}

	createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = VK_NULL_HANDLE;

	if (vkCreateSwapchainKHR(*m_pLogicalDevice, &createInfo, nullptr, &m_swapchain) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create swap chain!");
	}

	vkGetSwapchainImagesKHR(*m_pLogicalDevice, m_swapchain, &imageCount, nullptr);
	m_swapchainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(*m_pLogicalDevice, m_swapchain, &imageCount, m_swapchainImages.data());

	m_swapchainImageFormat = surfaceFormat.format;
	m_swapchainExtent = extent;
}

void Swapchain::createImageViews()
{
	//mDebugPrint("Creating image views...");
	m_swapchainImageViews.resize(m_swapchainImages.size());

	for (size_t i = 0; i < m_swapchainImages.size(); i++)
	{
		m_swapchainImageViews[i] = Image::createImageView(m_swapchainImages[i], m_swapchainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
	}
}

void Swapchain::recreateSwapchain(GLFWwindow* pWindow)
{
	int width = 0, height = 0;
	glfwGetFramebufferSize(pWindow, &width, &height);
	while (width == 0 || height == 0) {
		glfwGetFramebufferSize(pWindow, &width, &height);
		glfwWaitEvents();
	}

	vkDeviceWaitIdle(*m_pLogicalDevice);

	cleanup();

	createSwapchain();
	createImageViews();
	m_pBufferManager->getDepthBuffer()->createDepthResources();
	m_pBufferManager->getFramebuffer()->createFramebuffers();
}





void Swapchain::cleanup()
{
	m_pBufferManager->getFramebuffer()->cleanup();
	m_pBufferManager->getDepthBuffer()->cleanup();

	for (auto imageView : m_swapchainImageViews) {
		vkDestroyImageView(*m_pLogicalDevice, imageView, nullptr);
	}

	vkDestroySwapchainKHR(*m_pLogicalDevice, m_swapchain, nullptr);
}
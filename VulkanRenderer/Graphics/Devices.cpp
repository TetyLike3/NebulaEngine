#include "../VulkanRenderer.h"
#include "Window.h"

#include "Devices.h"



PhysicalDevice::PhysicalDevice() : m_pVkInstance(&VulkanEngine::getInstance()->m_vkInstance), m_pSurface(VulkanEngine::getInstance()->m_pWindow->getSurface()), m_pUtilities(Utilities::getInstance()) { pickPhysicalDevice(); };

VkPhysicalDevice* PhysicalDevice::pickPhysicalDevice()
{
	mDebugPrint("Picking physical device...");

	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(*m_pVkInstance, &deviceCount, nullptr);

	if (deviceCount == 0)
	{
		throw std::runtime_error("Failed to find GPUs with Vulkan support!");
	}

	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(*m_pVkInstance, &deviceCount, devices.data());

	for (const auto& device : devices)
	{
		if (isDeviceSuitable(device))
		{
			mDebugPrint("Found suitable device!");
			m_physicalDevice = device;
			break;
		}
	}

	if (m_physicalDevice == VK_NULL_HANDLE)
	{
		throw std::runtime_error("Failed to find a suitable GPU!");
	}

	return &m_physicalDevice;
}


bool PhysicalDevice::isDeviceSuitable(VkPhysicalDevice candidateDevice)
{
	mDebugPrint("Checking device suitability...");

	QueueFamilyIndices::sQueueFamilyIndices indices = QueueFamilyIndices::findQueueFamilies(candidateDevice, *m_pSurface);
	bool extensionsSupported = checkDeviceExtensionSupport(candidateDevice);

	mDebugPrint("Extensions supported: " + std::to_string(extensionsSupported));

	bool swapChainAdequate = false;
	if (extensionsSupported)
	{
		SwapChainSupportDetails swapChainSupport = querySwapChainSupport(candidateDevice);
		swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
	}
	
	VkPhysicalDeviceFeatures supportedFeatures;
	vkGetPhysicalDeviceFeatures(candidateDevice, &supportedFeatures);

	bool finalResult = indices.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
	mDebugPrint("Device suitable: " + std::to_string(finalResult));

	return finalResult;
}

// Returns whether the device supports all of the required extensions.
bool PhysicalDevice::checkDeviceExtensionSupport(VkPhysicalDevice candidateDevice)
{
	mDebugPrint("Checking device extension support...");

	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(candidateDevice, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(candidateDevice, nullptr, &extensionCount, availableExtensions.data());

	std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

	for (const auto& extension : availableExtensions)
	{
		requiredExtensions.erase(extension.extensionName);
	}

	// Print out the extensions that the device is missing
	std::string debugOutput = "";

	requiredExtensions.empty() ? debugOutput = "All extensions supported!" : debugOutput = "Device missing extensions: ";
	for (const auto& extension : requiredExtensions)
	{
		debugOutput += extension + ", ";
	}
	mDebugPrint(debugOutput);

	return requiredExtensions.empty();
}

// Returns the details of the swap chain support of the device.
SwapChainSupportDetails PhysicalDevice::querySwapChainSupport(VkPhysicalDevice physicalDevice)
{
	//mDebugPrint("Querying swap chain support...");

	SwapChainSupportDetails details;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, *m_pSurface, &details.capabilities);

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, *m_pSurface, &formatCount, nullptr);
	if (formatCount != 0)
	{
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, *m_pSurface, &formatCount, details.formats.data());
	}

	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, *m_pSurface, &presentModeCount, nullptr);
	if (presentModeCount != 0)
	{
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, *m_pSurface, &presentModeCount, details.presentModes.data());
	}

	return details;
}





LogicalDevice::LogicalDevice() : m_pPhysicalDevice(VulkanEngine::getInstance()->m_pPhysicalDevice), m_pSurface(VulkanEngine::getInstance()->m_pVkSurface), m_pWindow(VulkanEngine::getInstance()->m_pWindow->getWindow()), m_pUtilities(Utilities::getInstance()) { createLogicalDevice(); };

void LogicalDevice::createLogicalDevice()
{
	mDebugPrint("Creating logical device...");

	sSettings* pSettings = VulkanEngine::getInstance()->m_settings;

	sSettings::sDebugSettings pDebugSettings = pSettings->debugSettings;
	VkPhysicalDeviceFeatures deviceFeatures = pSettings->graphicsSettings.enabledFeatures;

	QueueFamilyIndices::sQueueFamilyIndices indices = QueueFamilyIndices::findQueueFamilies(*VulkanEngine::getInstance()->m_pPhysicalDevice->getVkPhysicalDevice(), *m_pSurface);

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

	float queuePriority = 1.0f;
	for (uint32_t queueFamily : uniqueQueueFamilies)
	{
		VkDeviceQueueCreateInfo queueCreateInfo{
			.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
			.queueFamilyIndex = queueFamily,
			.queueCount = 1,
			.pQueuePriorities = &queuePriority,
		};
		queueCreateInfos.push_back(queueCreateInfo);
	}

	VkDeviceCreateInfo createInfo{
		.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()),
		.pQueueCreateInfos = queueCreateInfos.data(),
		.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size()),
		.ppEnabledExtensionNames = deviceExtensions.data(),
		.pEnabledFeatures = &deviceFeatures
	};

	if (pDebugSettings.debugMode)
	{
		auto layerCount = static_cast<uint32_t>(pDebugSettings.validationLayers.size());
		layerCount == 1 ? mDebugPrint("Enabling 1 validation layer...") : mDebugPrint(std::format("Enabling {} validation layer(s)...", layerCount));
		createInfo.enabledLayerCount = layerCount;
		createInfo.ppEnabledLayerNames = pDebugSettings.validationLayers.data();
	}
	else {
		mDebugPrint("Validation layers not enabled.");
		createInfo.enabledLayerCount = 0;
	}

	if (vkCreateDevice(*VulkanEngine::getInstance()->m_pPhysicalDevice->getVkPhysicalDevice(), &createInfo, nullptr, &m_logicalDevice) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create logical device!");
	}

	vkGetDeviceQueue(m_logicalDevice, indices.graphicsFamily.value(), 0, &m_graphicsQueue);
	vkGetDeviceQueue(m_logicalDevice, indices.presentFamily.value(), 0, &m_presentQueue);
}

void LogicalDevice::cleanup()
{
	vkDestroyDevice(m_logicalDevice, nullptr);
}
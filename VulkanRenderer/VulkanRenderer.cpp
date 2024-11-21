#include "VulkanRenderer.h"


VulkanEngine* VulkanEngine::m_pInstance = nullptr;


VulkanEngine* VulkanEngine::getInstance()
{
	//nativeDebugPrint("Instance requested", true);
	if (m_pInstance == nullptr)
	{
		m_pInstance = new VulkanEngine();
		nativeDebugPrint("Instance created");
	}
	//nativeDebugPrint("Instance retrieved");
	return m_pInstance;
}

void VulkanEngine::destroyInstance()
{
	nativeDebugPrint("Instance destruction requested", true);
	if (m_pInstance != nullptr)
	{
		nativeDebugPrint("Cleaning up...");
		m_pInstance->m_state = VkEngineState::CLEANUP;
		m_pInstance->cleanup();

		nativeDebugPrint("Exiting...", true);
		m_pInstance->m_state = VkEngineState::EXIT;
		//delete m_pInstance;
		//m_pInstance = nullptr;
		//nativeDebugPrint("Instance destroyed");
	}
}


VulkanEngine::VulkanEngine() {}

void enableInputProcessing() {}

DebugMessenger* VulkanEngine::m_pDebugMessenger = nullptr;



void VulkanEngine::run(std::map<std::string, uint32_t> versions, sSettings* settings)
{
	m_pUtilities = Utilities::getInstance();

	mDebugPrint("Initialising...");
	m_state = VkEngineState::INIT;

	m_versions = versions;
	mDebugPrint("Engine version: " + m_pUtilities->getVkAPIVersionString(m_versions["engineVersion"]));

	m_settings = settings;
	m_MAX_FRAMES_IN_FLIGHT = m_settings->graphicsSettings.maxFramesInFlight;

	mDebugPrint("Maximum frames in flight: " + std::to_string(m_MAX_FRAMES_IN_FLIGHT) + "\n");

	Image::m_pGraphicsSettings = &m_settings->graphicsSettings;

	mDebugPrint("Creating window...");
	m_pWindow = new Window();

	mDebugPrint("Initialising Vulkan...");
	initVulkan();

	mDebugPrint("Initialisation successful, running...\n");
	m_state = VkEngineState::RUNNING;
	m_shouldRender = true;
	mainLoop();
}



void VulkanEngine::initVulkan()
{
	createInstance();

	// Debug messenger
	mDebugPrint("Creating debug messenger...");
	m_pDebugMessenger = new DebugMessenger();
	m_pDebugMessenger->setupDebugMessenger(&m_vkInstance, m_settings->debugSettings.debugMode);

	// Surface
	m_pWindow->createSurface();
	m_pVkSurface = m_pWindow->getSurface();

	// Devices
	m_pPhysicalDevice = new PhysicalDevice();
	m_pVkPhysicalDevice = m_pPhysicalDevice->getVkPhysicalDevice();

	validateSettings(); // Ensure device supports current settings

	m_pLogicalDevice = new LogicalDevice();
	m_pVkDevice = m_pLogicalDevice->getVkDevice();
	Image::m_pLogicalDevice = m_pVkDevice;

	// Buffer Manager
	m_pBufferManager = new BufferManager();
	Image::m_pBufferManager = m_pBufferManager;
	Model::m_pBufferManager = m_pBufferManager;

	// Command buffer
	m_pBufferManager->m_pCommandBuffer = new CommandBuffer(m_pBufferManager);

	// Swapchain
	m_pSwapchain = new Swapchain();
	m_pBufferManager->m_pSwapchain = m_pSwapchain;

	// Graphics pipeline
	m_pGraphicsPipeline = new GraphicsPipeline();
	m_pBufferManager->m_pGraphicsPipeline = m_pGraphicsPipeline->getGraphicsPipeline();
	m_pBufferManager->m_pRenderPass = m_pGraphicsPipeline->getRenderPass();
	m_pBufferManager->m_pDescriptorSetLayout = m_pGraphicsPipeline->getDescriptorSetLayout();
	m_pBufferManager->m_pPipelineLayout = m_pGraphicsPipeline->getVkPipelineLayout();

	// Create model
	Model* model1 = new Model("models/DTO_Crate.obj", "textures/DTO_Crate_Tex_Diffuse.png");
	Model* model2 = new Model("models/SF_Osprey.obj", "textures/DTO_Crate_Tex_Diffuse.png");
	Model* model3 = new Model("models/maxwell.obj", "textures/dingus_baseColor.jpeg");

	model2->changePosition(glm::vec3(0.0f, -2.0f, 0.0f));
	model2->changeScale(glm::vec3(.1f, .1f, .1f));
	model3->changePosition(glm::vec3(0.0f, 0.0f, 200.0f));
	model3->changeRotation(glm::vec3(0.0f, 180.0f, 0.0f));

	m_LoadedModels.push_back(model1);
	m_LoadedModels.push_back(model2);
	m_LoadedModels.push_back(model3);

	// Command buffer
	m_pBufferManager->m_pCommandBuffer->createCommandBuffers();


	// Initialise depth buffer
	m_pBufferManager->m_pDepthBuffer = new DepthBuffer(m_pBufferManager);

	// Initialise other buffers
	m_pBufferManager->m_pFramebuffer = new Framebuffer(m_pBufferManager);
	m_pBufferManager->m_pLoadedModels = &m_LoadedModels;

	// Sync objects
	m_pWindow->createSyncObjects();

	// Camera
	m_pCamera = new Camera();
	m_pCamera->m_cameraSensitivity = m_settings->controlSettings.cameraSensitivity;
	m_pCamera->m_cameraSpeed = m_settings->controlSettings.cameraSpeed;
	m_pWindow->setCamera(m_pCamera);
}

void VulkanEngine::createInstance()
{
	mDebugPrint("Creating Vulkan instance...");

	if (m_settings->debugSettings.debugMode && !checkValidationLayerSupport())
	{
		throw std::runtime_error("validation layers requested, but not available!");
	}

	mDebugPrint("Creating application info...");
	VkApplicationInfo appInfo{
		.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		.pApplicationName = "ElectrumGame",
		.applicationVersion = m_versions["gameVersion"],
		.pEngineName = "VulkanEngine",
		.engineVersion = m_versions["engineVersion"],
		.apiVersion = m_versions["apiVersion"]
	};

	VkInstanceCreateInfo createInfo{
		.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.pApplicationInfo = &appInfo
	};

	mDebugPrint("Creating instance extensions...");
	auto extensions = getRequiredExtensions();
	createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	createInfo.ppEnabledExtensionNames = extensions.data();

	if (m_settings->debugSettings.debugMode)
	{
		auto layerCount = static_cast<uint32_t>(m_settings->debugSettings.validationLayers.size());
		layerCount == 1 ? mDebugPrint("Enabling 1 validation layer...") : mDebugPrint(std::format("Enabling {} validation layer(s)...", layerCount));
		createInfo.enabledLayerCount = layerCount;
		createInfo.ppEnabledLayerNames = m_settings->debugSettings.validationLayers.data();
	}
	else
	{
		mDebugPrint("Validation layers not enabled.");
		createInfo.enabledLayerCount = 0;
	}

	if (vkCreateInstance(&createInfo, nullptr, &m_vkInstance) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create instance!");
	}
}




void VulkanEngine::mainLoop()
{
	m_pWindow->setVBOCount(m_pBufferManager->getVertexBuffers()->size());
	m_pWindow->mainLoop();
}

void VulkanEngine::rebuildGraphicsPipeline() {
	m_shouldRender = false;

	mDebugPrint("Rebuilding graphics pipeline...");

	//m_pBufferManager->m_pFramebuffer->cleanup();
	m_pBufferManager->m_pCommandBuffer->cleanup();

	m_pGraphicsPipeline->cleanup();

	m_pSwapchain->cleanup();

	m_pBufferManager->m_pCommandBuffer = new CommandBuffer(m_pBufferManager);

	m_pSwapchain = new Swapchain();
	m_pBufferManager->m_pSwapchain = m_pSwapchain;

	m_pGraphicsPipeline = new GraphicsPipeline();
	m_pBufferManager->m_pGraphicsPipeline = m_pGraphicsPipeline->getGraphicsPipeline();
	m_pBufferManager->m_pRenderPass = m_pGraphicsPipeline->getRenderPass();
	m_pBufferManager->m_pDescriptorSetLayout = m_pGraphicsPipeline->getDescriptorSetLayout();
	m_pBufferManager->m_pPipelineLayout = m_pGraphicsPipeline->getVkPipelineLayout();

	m_pBufferManager->m_pFramebuffer = new Framebuffer(m_pBufferManager);

	m_pBufferManager->m_pCommandBuffer->createCommandBuffers();

	m_shouldRender = true;
}


void VulkanEngine::cleanup()
{
	vkDeviceWaitIdle(*m_pVkDevice);

	mDebugPrint("Cleaning up graphics pipeline...");
	m_pGraphicsPipeline->cleanup();
	delete m_pGraphicsPipeline;

	mDebugPrint("Cleaning up loaded models...");
	for (Model* model : m_LoadedModels)
	{
		mDebugPrint("cleaning up model");
		model->cleanup();
		delete model;
	}

	mDebugPrint("Cleaning up sync objects...");
	m_pWindow->cleanupSyncObjects();

	//mDebugPrint("Cleaning up buffers...");
	//m_pBufferManager->cleanup();

	mDebugPrint("Cleaning up command buffer...");
	m_pBufferManager->m_pCommandBuffer->cleanup();
	delete m_pBufferManager->m_pCommandBuffer;

	mDebugPrint("Cleaning up swapchain...");
	m_pSwapchain->cleanup();
	delete m_pSwapchain;

	// For some reason it shits itself if I try to clean up the texture image??
	// 
	//mDebugPrint("Cleaning up texture image...");
	//m_pTextureImage->cleanup();
	//delete m_pTextureImage;

	delete m_pBufferManager;

	mDebugPrint("Cleaning up logical device...");
	m_pLogicalDevice->cleanup();
	delete m_pLogicalDevice;

	if (m_settings->debugSettings.debugMode)
	{
		mDebugPrint("Cleaning up debug messenger...");
		m_pDebugMessenger->cleanup();
		delete m_pDebugMessenger;
	}

	mDebugPrint("Cleaning up surface...");
	m_pWindow->cleanupSurface();

	mDebugPrint("Cleaning up Vulkan instance...");
	vkDestroyInstance(m_vkInstance, nullptr);

	mDebugPrint("Cleaning up window...");
	m_pWindow->cleanupWindow();
	delete m_pWindow;
}




bool VulkanEngine::checkValidationLayerSupport()
{
	mDebugPrint("Checking validation layer support...");

	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const char* layerName : m_settings->debugSettings.validationLayers)
	{
		bool layerFound = false;

		for (const auto& layerProperties : availableLayers)
		{
			if (strcmp(layerName, layerProperties.layerName) == 0)
			{

				mDebugPrint(std::format("Found validation layer: {}", layerName));
				layerFound = true;
				break;
			}
		}

		if (!layerFound)
		{
			mDebugPrint("Validation layer not found");
			return false;
		}
	}

	return true;
}

std::vector<const char*> VulkanEngine::getRequiredExtensions()
{
	mDebugPrint("Getting required extensions...");
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

	if (m_settings->debugSettings.debugMode)
	{
		mDebugPrint("Enabling validation layer extensions...");
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	return extensions;
}

void VulkanEngine::validateSettings()
{
	mDebugPrint("Validating settings...");

	int settingsChanged = 0;

	VkPhysicalDeviceProperties properties{};
	vkGetPhysicalDeviceProperties(*m_pVkPhysicalDevice, &properties);
	VkPhysicalDeviceFeatures features{};
	vkGetPhysicalDeviceFeatures(*m_pVkPhysicalDevice, &features);

	// Check if the device supports anisotropic filtering
	if (properties.limits.maxSamplerAnisotropy < m_settings->graphicsSettings.anisotropyLevel)
	{
		mDebugPrint(std::format("Anisotropic filtering level of x{} is not supported by the device. Setting to x{}.", m_settings->graphicsSettings.anisotropyLevel, properties.limits.maxSamplerAnisotropy));
		m_settings->graphicsSettings.anisotropyLevel = properties.limits.maxSamplerAnisotropy;
		settingsChanged++;
	}

	// Check if device supports wireframe rendering
	if (!features.fillModeNonSolid)
	{
		mDebugPrint("Wireframe rendering is not supported by the device. Setting to false.");
		m_settings->graphicsSettings.wireframe = false;
		settingsChanged++;
	}
	if (!features.wideLines)
	{
		mDebugPrint("Wide lines are not supported by the device. Setting wireframe thickness to 1px.");
		m_settings->graphicsSettings.wireframeThickness = 1.0f;
		settingsChanged++;
	}

	if (!features.sampleRateShading) {
		mDebugPrint("Sample rate shading is not supported by the device. Disabling multisampling.");
		m_settings->graphicsSettings.multisampling = false;
		settingsChanged++;
	}

	settingsChanged != 1 ? mDebugPrint(std::format("Settings validated with {} changes.", settingsChanged)) : mDebugPrint("Settings validated with 1 change.");
}

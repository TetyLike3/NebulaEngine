#include "../VulkanRenderer.h"
#include "Devices.h"
#include "Buffers.h"
#include "Swapchain.h"

#include "Window.h"




Window::Window() : m_pVkInstance(&VulkanEngine::getInstance()->m_vkInstance), m_MAX_FRAMES_IN_FLIGHT(VulkanEngine::getInstance()->m_MAX_FRAMES_IN_FLIGHT), m_pUtilities(Utilities::getInstance()),
					m_pGraphicsSettings(&VulkanEngine::getInstance()->m_settings->graphicsSettings), m_pShouldRender(VulkanEngine::getInstance()->getShouldRender())
{
	initWindow();
};

void Window::initWindow()
{
	mDebugPrint("Initializing window...");

	sSettings::sWindowSettings windowSettings = VulkanEngine::getInstance()->m_settings->windowSettings;

	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	//glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	m_pWindow = glfwCreateWindow(windowSettings.width, windowSettings.height, windowSettings.title, nullptr, nullptr);
	glfwSetWindowUserPointer(m_pWindow, this);
	glfwSetFramebufferSizeCallback(m_pWindow, framebufferResizeCallback);

	if (m_pGraphicsSettings->vsync) m_pGraphicsSettings->maxFramerate = 60; // TODO: Check for display's refresh rate
	if (m_pGraphicsSettings->maxFramerate > 0) m_renderTargetDelta = (1.0f / (float)m_pGraphicsSettings->maxFramerate); else m_renderTargetDelta = 0.0f;
}

void Window::framebufferResizeCallback(GLFWwindow* window, int width, int height)
{
	auto app = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
	app->m_framebufferResized = true;
	//app->drawFrame(); // Don't do this or it'll break the window when one of the coordinates is 0
}


void Window::createSurface()
{
	mDebugPrint("Creating surface...");

	if (glfwCreateWindowSurface(*m_pVkInstance, m_pWindow, nullptr, &m_surface) != VK_SUCCESS) {
		throw std::runtime_error("failed to create window surface!");
	}
}

void Window::createSyncObjects()
{
	mDebugPrint("Creating sync objects...");

	// Define these variables here since we need to use them from now on
	m_pLogicalDevice = VulkanEngine::getInstance()->m_pLogicalDevice->getVkDevice();
	m_pGraphicsQueue = VulkanEngine::getInstance()->m_pLogicalDevice->getGraphicsQueue();
	m_pSwapchain = VulkanEngine::getInstance()->m_pSwapchain;
	m_pCommandBuffer = VulkanEngine::getInstance()->m_pBufferManager->getCommandBuffer();

	// Resize the vectors to the correct size
	m_imageAvailableSemaphores.resize(m_MAX_FRAMES_IN_FLIGHT);
	m_renderFinishedSemaphores.resize(m_MAX_FRAMES_IN_FLIGHT);
	m_inFlightFences.resize(m_MAX_FRAMES_IN_FLIGHT);

	VkSemaphoreCreateInfo semaphoreInfo{
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
	};

	VkFenceCreateInfo fenceInfo{
		.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
		.flags = VK_FENCE_CREATE_SIGNALED_BIT
	};

	for (size_t i = 0; i < m_MAX_FRAMES_IN_FLIGHT; i++)
	{
		if (vkCreateSemaphore(*m_pLogicalDevice, &semaphoreInfo, nullptr, &m_imageAvailableSemaphores[i]) != VK_SUCCESS ||
			vkCreateSemaphore(*m_pLogicalDevice, &semaphoreInfo, nullptr, &m_renderFinishedSemaphores[i]) != VK_SUCCESS ||
			vkCreateFence(*m_pLogicalDevice, &fenceInfo, nullptr, &m_inFlightFences[i]) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create synchronization objects for a frame!");
		}
	}
}


void Window::mainLoop()
{
	while (!glfwWindowShouldClose(m_pWindow))
	{
		glfwPollEvents();
		drawFrame();

		// Print FPS
		calculateFPS();
	}
	mDebugPrint("Window closed, waiting for device idle...");

	vkDeviceWaitIdle(*m_pLogicalDevice);
}


void Window::drawFrame()
{
	if (!m_pShouldRender) return;
	double currentDelta = glfwGetTime() - m_renderLastTime;
	if (currentDelta < m_renderTargetDelta) return;

	uint32_t imageIndex;

	std::vector<VkCommandBuffer> commandBuffers = *m_pCommandBuffer->getCommandBuffers();

	vkWaitForFences(*m_pLogicalDevice, 1, &m_inFlightFences[m_currentFrame], VK_TRUE, UINT64_MAX);

	m_cpuWorkTime = glfwGetTime() - m_renderLastTime;
	double timeAfterFences = glfwGetTime();

	VkResult result = vkAcquireNextImageKHR(*m_pLogicalDevice, *m_pSwapchain->getSwapchain(), UINT64_MAX, m_imageAvailableSemaphores[m_currentFrame], VK_NULL_HANDLE, &imageIndex);

	// Ensure swapchain quality
	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		m_pSwapchain->recreateSwapchain(m_pWindow);
		return;
	}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
		throw std::runtime_error("failed to acquire swap chain image!");
	}

	updateUniformBuffers(m_currentFrame); // Perform translations

	vkResetFences(*m_pLogicalDevice, 1, &m_inFlightFences[m_currentFrame]);

	vkResetCommandBuffer(commandBuffers[m_currentFrame], 0);
	m_pCommandBuffer->recordCommandBuffer(commandBuffers[m_currentFrame],imageIndex);


	VkSemaphore waitSemaphores[] = { m_imageAvailableSemaphores[m_currentFrame] };
	VkSemaphore signalSemaphores[] = { m_renderFinishedSemaphores[m_currentFrame] };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

	VkSubmitInfo submitInfo{
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = waitSemaphores,
		.pWaitDstStageMask = waitStages,
		.commandBufferCount = 1,
		.pCommandBuffers = &commandBuffers[m_currentFrame],
		.signalSemaphoreCount = 1,
		.pSignalSemaphores = signalSemaphores
	};

	if (vkQueueSubmit(*m_pGraphicsQueue, 1, &submitInfo, m_inFlightFences[m_currentFrame]) != VK_SUCCESS) {
		throw std::runtime_error("failed to submit draw command buffer!");
	}


	VkSwapchainKHR swapChains[] = { *m_pSwapchain->getSwapchain() };

	VkPresentInfoKHR presentInfo{
		.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = signalSemaphores,
		.swapchainCount = 1,
		.pSwapchains = swapChains,
		.pImageIndices = &imageIndex,
		.pResults = nullptr // Optional
	};

	result = vkQueuePresentKHR(*m_pGraphicsQueue, &presentInfo);

	// Ensure swapchain quality
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result != VK_SUBOPTIMAL_KHR || m_framebufferResized)
	{
		m_framebufferResized = false;
		m_pSwapchain->recreateSwapchain(m_pWindow);
	}
	else if (result != VK_SUCCESS) {
		throw std::runtime_error("failed to present swap chain image!");
	}

	m_frameCounter++;
	m_currentFrame = (m_currentFrame + 1) % m_MAX_FRAMES_IN_FLIGHT;

	m_gpuDrawTime = glfwGetTime() - timeAfterFences;

	m_renderLastTime = glfwGetTime();
}

void Window::updateUniformBuffers(uint32_t currentImage)
{
	VulkanEngine* pVulkanEngine = VulkanEngine::getInstance();
	using std::chrono::high_resolution_clock, std::chrono::duration, std::chrono::seconds;

	static auto startTime = high_resolution_clock::now();

	auto currentTime = high_resolution_clock::now();
	float time = duration<float, seconds::period>(currentTime - startTime).count();


	VkExtent2D swapchainExtent = *m_pSwapchain->getSwapchainExtent();

	for (Model* model : pVulkanEngine->m_LoadedModels) {
		UniformBufferObject::sUniformBufferObject ubo{
			.model = model->getTransform(),
			.view = glm::lookAt(m_pCamera->m_cameraPosition, m_pCamera->m_cameraPosition + m_pCamera->m_cameraFront, m_pCamera->m_cameraUp),
			.proj = glm::perspective(glm::radians(70.0f), (float)swapchainExtent.width / swapchainExtent.height, m_pGraphicsSettings->nearClip, m_pGraphicsSettings->farClip)
		};
		ubo.proj[1][1] *= -1; // Flip the y axis to account for Vulkan's inverted y axis

		std::vector<void*> uniformBuffersMapped = *model->m_pUniformBufferObject->getUniformBuffersMapped(); // still fuck this piece of code in particular
		memcpy(uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
	}
}


void Window::calculateFPS()
{
	using std::string, std::to_string;
	double current = glfwGetTime();
	double delta = current - m_lastTime;

	if (delta >= 1) // Wait 1 second
	{
		// Print the FPS with a precision of 2 d.p.
		string fpsString = to_string(m_frameCounter/delta);
		string cpuWaitString = to_string(m_cpuWorkTime*1000);
		string gpuDrawString = to_string((m_gpuDrawTime*1000));
		string vboCount = to_string(m_vboCount);
		mDebugPrint(std::format("\x1b[36;49m{}", "FPS (current): " + fpsString.substr(0, fpsString.find(".") + 3)));
		mDebugPrint(std::format("\x1b[33;49m{}", "CPU work (ms): " + cpuWaitString.substr(0, cpuWaitString.find(".") + 3)));
		mDebugPrint(std::format("\x1b[33;49m{}", "GPU draw (ms): " + gpuDrawString.substr(0, gpuDrawString.find(".") + 3)));
		mDebugPrint(std::format("\x1b[36;49m{}", "VBO count: " + vboCount));

		m_frameCounter = 0;
		m_lastTime = current;
	}
}




void Window::cleanupSyncObjects()
{
	for (size_t i = 0; i < m_MAX_FRAMES_IN_FLIGHT; i++) {
		vkDestroySemaphore(*m_pLogicalDevice, m_renderFinishedSemaphores[i], nullptr);
		vkDestroySemaphore(*m_pLogicalDevice, m_imageAvailableSemaphores[i], nullptr);
		vkDestroyFence(*m_pLogicalDevice, m_inFlightFences[i], nullptr);
	}
}

void Window::cleanupSurface()
{
	vkDestroySurfaceKHR(*m_pVkInstance, m_surface, nullptr);
}

void Window::cleanupWindow()
{
	glfwDestroyWindow(m_pWindow);
	glfwTerminate();
}

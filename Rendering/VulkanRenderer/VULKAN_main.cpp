#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <iostream>
#include <map>
#include <thread>

#include "Rendering/VulkanRenderer/VulkanRenderer.h"
#include "Rendering/VulkanRenderer/Utilities/Utilities.h"

const std::map<std::string, uint32_t> versions = {
	{ "engineVersion", VK_MAKE_API_VERSION(0,0,9,0) },
};

sSettings settings{
	.windowSettings {
		.title = "NebulaEngine",
		.width = 1280,
		.height = 720,
	},
	.debugSettings {
		#ifdef NDEBUG
		.debugMode = false,
		.validationLayers = {},
		.enableValidationLayers = false,
		#else
		.debugMode = true,
		.validationLayers = {
			"VK_LAYER_KHRONOS_validation"
		},
		.enableValidationLayers = true
		#endif
	},
	.graphicsSettings {
		.maxFramesInFlight = 1,
		.enabledFeatures = {
			.sampleRateShading = true,
			.fillModeNonSolid = true,
			.wideLines = true,
			.samplerAnisotropy = true
		},
		.tripleBuffering = true,
		.vsync = false,
		.maxFramerate = 0,
		.rasterizerDepthClamp = false,
		.wireframe = false,
		.wireframeThickness = 8.0f,
		.multisampling = true,
		.anisotropicFiltering = true,
		.anisotropyLevel = 16.0f,
		.nearClip = 0.1f,
		.farClip = 1000.0f
	},
	.controlSettings {
		.cameraSensitivity = 2.0f,
		.cameraSpeed = 0.1f
	}
};


void runVulkanEngine(VulkanEngine *pVulkanEngine)
{
	try
	{
		pVulkanEngine->run(versions, &settings);
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		system("pause");
	}

	VulkanEngine::destroyInstance();
}


bool keyW = false;
bool keyA = false;
bool keyS = false;
bool keyD = false;
double lastMouseX = settings.windowSettings.width / 2;
double lastMouseY = settings.windowSettings.height / 2;
bool firstMouseInput = true;

void enableInputProcessing(VulkanEngine *pVulkanEngine)
{
	Window* pWindow = pVulkanEngine->getWindow();
	GLFWwindow* pGLFWWindow = pWindow->getWindow();
	Camera* pCamera = pVulkanEngine->getCamera();

	glfwSetInputMode(pGLFWWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	while (pVulkanEngine->getState() == VkEngineState::RUNNING) {
		// Process inputs
		if (glfwGetKey(pGLFWWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
			glfwSetWindowShouldClose(pGLFWWindow, GLFW_TRUE);
		}

		if (glfwGetKey(pGLFWWindow, GLFW_KEY_W) == GLFW_PRESS) keyW = true;
		if (glfwGetKey(pGLFWWindow, GLFW_KEY_W) == GLFW_RELEASE) keyW = false;
		if (glfwGetKey(pGLFWWindow, GLFW_KEY_A) == GLFW_PRESS) keyA = true;
		if (glfwGetKey(pGLFWWindow, GLFW_KEY_A) == GLFW_RELEASE) keyA = false;
		if (glfwGetKey(pGLFWWindow, GLFW_KEY_S) == GLFW_PRESS) keyS = true;
		if (glfwGetKey(pGLFWWindow, GLFW_KEY_S) == GLFW_RELEASE) keyS = false;
		if (glfwGetKey(pGLFWWindow, GLFW_KEY_D) == GLFW_PRESS) keyD = true;
		if (glfwGetKey(pGLFWWindow, GLFW_KEY_D) == GLFW_RELEASE) keyD = false;

		if (glfwGetKey(pGLFWWindow, GLFW_KEY_P) == GLFW_PRESS) {
			pVulkanEngine->m_settings->graphicsSettings.wireframe = !pVulkanEngine->m_settings->graphicsSettings.wireframe;
			pVulkanEngine->rebuildGraphicsPipeline();
		}

		glm::vec3 dirInput = glm::vec3(0.0f);
		if (keyW) dirInput.z = 1;
		if (keyA) dirInput.x = -1;
		if (keyS) dirInput.z = -1;
		if (keyD) dirInput.x = 1;

		glm::vec3 angInput = glm::vec3(0.0f);
		double currentMouseX, currentMouseY;
		glfwGetCursorPos(pGLFWWindow, &currentMouseX, &currentMouseY);
		if (firstMouseInput) {
			lastMouseX = currentMouseX;
			lastMouseY = currentMouseY;
			firstMouseInput = false;
		}

		angInput.y = (float)(currentMouseX - lastMouseX);
		angInput.x = (float)(lastMouseY - currentMouseY);
		lastMouseX = currentMouseX;
		lastMouseY = currentMouseY;

		pCamera->move(dirInput, angInput);

		// Sleep for a bit to reduce CPU usage
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
}



//int main()
//{
//	VulkanEngine* pVulkanEngine = VulkanEngine::getInstance();
//
//	Utilities::EngineWorkingDirectory = std::filesystem::current_path().string();
//	Utilities::compileShaders("shaders");
//
//	std::thread renderThread(runVulkanEngine, pVulkanEngine);
//
//	// Wait for rendering engine to start running
//	while (pVulkanEngine->getState() != VkEngineState::RUNNING) {
//		std::this_thread::sleep_for(std::chrono::milliseconds(100));
//	}
//
//	// Allow inputs to be processed
//	std::thread inputThread(enableInputProcessing, pVulkanEngine);
//
//	// Wait for rendering engine to clean up before exiting
//	while (pVulkanEngine->getState() != VkEngineState::EXIT) {
//		std::this_thread::sleep_for(std::chrono::milliseconds(100));
//	}
//
//	renderThread.join();
//	inputThread.join();
//
//	std::cout << "Program ended successfully.\n";
//	system("pause");
//
//	return EXIT_SUCCESS;
//}
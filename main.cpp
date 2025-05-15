
#include <iostream>
#include <map>
#include <thread>
#include <string>

#include "Rendering/OpenGLRenderer/OpenGLRenderer.h"

Settings *settings = new Settings();

bool keyW = false;
bool keyA = false;
bool keyS = false;
bool keyD = false;
double lastMouseX = settings->window.windowWidth / 2;
double lastMouseY = settings->window.windowHeight / 2;
bool firstMouseInput = true;

void enableInputProcessing(OpenGLRenderer* pRenderer)
{
	Window* pWindow = pRenderer->getWindow();
	GLFWwindow* pGLFWWindow = pWindow->getWindow();
	Camera* pCamera = pRenderer->getCamera();

	glfwSetInputMode(pGLFWWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	while (pRenderer->getState() == OpenGLRendererState::RUNNING) {
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

void runRenderer(OpenGLRenderer* pRenderer) {
	try
	{
		pRenderer->init(settings);
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		system("pause");
	}

	OpenGLRenderer::destroyInstance();
}

int main() {
	OpenGLRenderer* pOpenGLRenderer = OpenGLRenderer::getInstance();

	settings->window.windowWidth = 1920;
	settings->window.windowHeight = 1080;

	std::cout << "Starting render thread...\n";
	std::thread renderThread(runRenderer, pOpenGLRenderer);
	
	// Wait for rendering engine to start running
	while (pOpenGLRenderer->getState() != OpenGLRendererState::RUNNING) {
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	std::cout << "Starting input thread...\n";
	// Allow inputs to be processed
	std::thread inputThread(enableInputProcessing, pOpenGLRenderer);
	
	// Wait for rendering engine to clean up before exiting
	while (pOpenGLRenderer->getState() != OpenGLRendererState::EXIT) {
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
	
	renderThread.join();
	inputThread.join();
	
	std::cout << "Program ended successfully.\n";
	system("pause");
	
	return EXIT_SUCCESS;
}
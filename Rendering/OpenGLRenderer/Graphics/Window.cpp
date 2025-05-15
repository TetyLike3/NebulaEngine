#include "Window.h"

#include "../Models/Camera.h"

Window::Window() {}


void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}

int Window::init(Settings::Window windowSettings) {
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	m_pWindow = glfwCreateWindow(windowSettings.windowWidth, windowSettings.windowHeight, windowSettings.windowTitle, NULL, NULL);
	if (m_pWindow == NULL) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(m_pWindow);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	glViewport(0, 0, windowSettings.windowWidth, windowSettings.windowHeight);
	glfwSetFramebufferSizeCallback(m_pWindow, framebufferResizeCallback);
}

void Window::updateLoop() {
	drawFrame();
}

void Window::cleanup() {
	glfwTerminate();
}


void Window::drawFrame() {
}

#include "OpenGLRenderer.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

OpenGLRenderer* OpenGLRenderer::m_pInstance = nullptr;

OpenGLRenderer::OpenGLRenderer() {}

OpenGLRenderer* OpenGLRenderer::getInstance()
{
	if (m_pInstance == nullptr)
	{
		m_pInstance = new OpenGLRenderer();
		nativeDebugPrint("Instance created");
	}
	return m_pInstance;
}

void OpenGLRenderer::destroyInstance()
{
	nativeDebugPrint("Instance destruction requested", true);
	if (m_pInstance != nullptr)
	{
		if (m_pInstance->getState() != OpenGLRendererState::CLEANUP) {
			m_pInstance->cleanup();
		}

		nativeDebugPrint("Exiting...", true);
		m_pInstance->m_state = OpenGLRendererState::EXIT;
	}
}

std::string shadersPath = "Rendering/shaders";
std::vector<BaseShader> LoadedShaders = {};

void OpenGLRenderer::loadShaders() {
	using std::string, std::vector, std::filesystem::directory_entry;

	vector<directory_entry> shaderSources = Utilities::getFoldersInFolder(shadersPath);

	for (const auto& shaderSource : shaderSources) {
		string shaderSourcePath = shaderSource.path().string();
		mDebugPrint("Loading shader source: " + shaderSourcePath);
		BaseShader newShader = BaseShader(shaderSourcePath);
		LoadedShaders.push_back(newShader);
	}
}

std::vector<unsigned int> VAOs = {};
std::vector<unsigned int> VBOs = {};
std::vector<BaseModel> LoadedModels = {};

int OpenGLRenderer::init(Settings* initSettings) {
	m_state = OpenGLRendererState::INIT;
	nativeDebugPrint("Initializing...", true);
	m_currentSettings = *initSettings;

	mDebugPrint("Instantiating window...");
	glfwInit();
	m_pWindow = new Window();
	m_pWindow->init(initSettings->window);

	mDebugPrint("Loading shaders...");
	// Prepare shaders
	loadShaders();

	mDebugPrint("Creating memory buffers...");
	// Set up vertex data
	sVertexData vertexData[4] = {
		{ glm::vec3(0.5f, 0.5f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(1.0f, 1.0f) },
		{ glm::vec3(0.5f, -0.5f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(1.0f, 0.0f) },
		{ glm::vec3(-0.5f, -0.5f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 0.0f) },
		{ glm::vec3(-0.5f, 0.5f, 0.0f), glm::vec3(1.0f, 1.0f, 0.0f), glm::vec2(0.0f, 1.0f) },
	};

	unsigned int indices[] = {
		0, 1, 3,
		1, 2, 3
	};

	unsigned int VAO, VBO, EBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	VAOs.push_back(VAO);
	VBOs.push_back(VBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertexData), vertexData, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	glGenBuffers(1, &m_defaultUBO);
	glBindBuffer(GL_UNIFORM_BUFFER, m_defaultUBO);
	glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), NULL, GL_STATIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_defaultUBO);

	mDebugPrint("Creating test model...");
	BaseTexture testTexture = BaseTexture("./Rendering/textures/image.png");
	BaseModel testModel = BaseModel(LoadedShaders[0], testTexture, VAOs[0]);
	LoadedModels.push_back(testModel); // asshole

	mDebugPrint("Instantiating camera...");
	m_pCamera = new Camera();
	m_pCamera->setCameraSpeed(m_currentSettings.controls.cameraSpeed);
	m_pCamera->setCameraSensitivity(m_currentSettings.controls.cameraSensitivity);
	
	mDebugPrint("Initialisation complete");

	m_startRenderTime = glfwGetTime();
	mainLoop();

	return 0;
}


void OpenGLRenderer::mainLoop() {
	m_state = OpenGLRendererState::RUNNING;
	double fpsPrintDelta = 0;
	GLFWwindow* windowInstance = m_pWindow->getWindow();
	while (!glfwWindowShouldClose(windowInstance))
	{
		m_deltaRenderTime = glfwGetTime() - m_elapsedRenderTime;
		m_elapsedRenderTime = glfwGetTime() - m_startRenderTime;
		if (fpsPrintDelta > 30) {
			fpsPrintDelta = 0;
			mDebugPrint(std::to_string(m_elapsedRenderTime) + " elapsed | " + std::to_string(m_deltaRenderTime) + " delta | " + Utilities::calculateFPS(m_deltaRenderTime, 2) + "FPS");
		};
		fpsPrintDelta += m_deltaRenderTime;

		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		LoadedShaders[0].setUniformVec4("ourColor", 0.0f, static_cast<float>(sin(m_elapsedRenderTime)), 0.0f, 1.0f);
		updateUniformBuffers();

		// Draw each loaded model
		for (int i = 0; i < LoadedModels.size(); i++) {
			LoadedModels[i].DrawModel();
		}

		glfwSwapBuffers(windowInstance);
		glfwPollEvents();
	}

	m_pInstance->cleanup();
}

void OpenGLRenderer::updateUniformBuffers() {
	float aspectRatio = m_currentSettings.window.windowWidth / static_cast<float>(m_currentSettings.window.windowHeight);
	glm::mat4 vpData[2] = {
		m_pCamera->getViewMatrix(),
		glm::perspective(glm::radians(70.0f),aspectRatio,m_currentSettings.graphics.clipPlaneNear,m_currentSettings.graphics.clipPlaneFar)
	};
	glBindBuffer(GL_UNIFORM_BUFFER, m_defaultUBO);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(vpData), &vpData);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void OpenGLRenderer::cleanup() {
	m_state = OpenGLRendererState::CLEANUP;
	nativeDebugPrint("Cleaning up...", true);
	glfwTerminate();
}
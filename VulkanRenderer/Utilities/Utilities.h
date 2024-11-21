#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan_core.h>

#include <string>
#include <iostream>
#include <chrono>
#include <format>
#include <fstream>
#include <iterator>
#include <filesystem>

// Macro to print debug messages with class name argument autofilled
#define mDebugPrint(x) m_pUtilities->debugPrint(x, this)


struct sSettings {
	struct sWindowSettings {
		const char* title = "NebulaEngine"; // Window title.
		uint32_t width = 1280; // Window width.
		uint32_t height = 720; // Window height.
	} windowSettings;
	struct sDebugSettings {
		#ifdef NDEBUG
			bool debugMode = false;
		#else
			bool debugMode = true;
		#endif
		std::vector<const char*> validationLayers = { // Validation layers to enable.
			"VK_LAYER_KHRONOS_validation"
		};
		bool enableValidationLayers = true; // Enable validation layers.
	} debugSettings;
	struct sGraphicsSettings {
		int maxFramesInFlight = 2; // How many frames the CPU can queue for rendering at once.
		VkPhysicalDeviceFeatures enabledFeatures = {}; // Physical device features to enable.
		bool tripleBuffering = true; // Enable Triple buffering.
		bool vsync = true; // Enable VSync.
		int maxFramerate = 0; // Maximum frame rate of the engine (0 for unlimited).
		bool rasterizerDepthClamp = false; // Enable depth clamping.
		bool wireframe = true; // Enable Wireframe rendering.
		float wireframeThickness = 2.0f; // Thickness of wireframes when using Wireframe rendering.
		bool multisampling = false; // Enable Multisampling.
		VkBool32 anisotropicFiltering = false; // Enable Anisotropic filtering.
		float anisotropyLevel = 4.0f; // Anisotropy level (1.0f = no anisotropy).
		float nearClip = 0.1f; // Near clipping plane.
		float farClip = 1000.0f; // Far clipping plane.
	} graphicsSettings;
	struct sControlSettings {
		float cameraSensitivity = .1f; // Sensitivity of the camera movement.
		float cameraSpeed = 0.05f; // Speed of the camera movement.
	} controlSettings;
};


// Singleton class for utility functions
class Utilities
{
public:
	static Utilities* getInstance();

	template<typename CLASSNAME>
	// Prints a debug message with the class name and timestamp.
	inline void debugPrint(std::string message, CLASSNAME* that) { iDebugPrint(message, std::string(typeid(that).name())); };
	inline static void debugPrint(std::string message, std::string className) { m_pInstance->iDebugPrint(message, className); };

	inline std::string getVkAPIVersionString(uint32_t version)
	{
		using std::string, std::to_string;
		return to_string(VK_API_VERSION_MAJOR(version)) += string(".") += to_string(VK_API_VERSION_MINOR(version)) += string(".") += to_string(VK_API_VERSION_PATCH(version));
	};

	// Generates a timestamp in the format HH:MM:SS.mmm
	std::string generateTimestamp_HH_MM_SS_mmm();


	std::vector<char> readFile(const std::string& filename);
	static std::vector<std::filesystem::directory_entry> getFilesInFolder(std::filesystem::path folderPath);
	static std::vector<std::filesystem::directory_entry> getFilesOfExtInFolder(std::filesystem::path folderPath, std::string ext);

	static std::vector<std::string>* pCompiledVertShaders;
	static std::vector<std::string>* pCompiledFragShaders;
	static void compileShaders(std::filesystem::path folderPath);
private:
	Utilities();

	static Utilities* m_pInstance;

	static std::string m_lastClassPrinted;
	static std::string m_lastMessagePrinted;


	inline void iDebugPrint(std::string message, std::string className)
	{
		using std::string, std::format, std::cerr, std::setw;

		string timestamp = generateTimestamp_HH_MM_SS_mmm();

		// Truncate the "* __ptr64" at the end of the class name
		if (className.find("* __ptr64") != string::npos)
			className = className.substr(0, className.find("* __ptr64"));

		// Print class name only once
		if (m_lastClassPrinted != className)
		{
			m_lastClassPrinted = className;
			cerr << format("\nVulkanEngine \x1b[31;49;1mDEBUG\x1b[39;49m - \x1b[32;49m{}\x1b[39;49m -\n", m_lastClassPrinted);
		};

		m_lastMessagePrinted = format("\x1b[36;49m[{}]\x1b[39;49m - {}\n", generateTimestamp_HH_MM_SS_mmm(), message);
		cerr << m_lastMessagePrinted;
	}
};


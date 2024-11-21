#include "Utilities.h"
#include <windows.h>

Utilities* Utilities::m_pInstance = nullptr;

Utilities* Utilities::getInstance()
{
	if (m_pInstance == nullptr)
	{
		m_pInstance = new Utilities();
	}

	return m_pInstance;
};

Utilities::Utilities() {};


std::string Utilities::m_lastClassPrinted = "";
std::string Utilities::m_lastMessagePrinted = "";


std::string Utilities::generateTimestamp_HH_MM_SS_mmm()
{
	using namespace std::chrono;

	// get current time
	auto now = system_clock::now();

	// get number of milliseconds for the current second
	// (remainder after division into seconds)
	auto ms = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;

	// convert to std::time_t in order to convert to std::tm (broken time)
	auto timer = system_clock::to_time_t(now);

	// convert to broken time
#pragma warning(suppress : 4996)
	std::tm bt = *std::localtime(&timer);

	std::ostringstream oss;

	oss << std::put_time(&bt, "%H:%M:%S"); // HH:MM:SS
	oss << '.' << std::setfill('0') << std::setw(3) << ms.count();

	return oss.str();
};



std::vector<char> Utilities::readFile(const std::string& filename)
{
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		throw std::runtime_error("failed to open file!");
	};

	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);
	file.close();

	return buffer;
};

std::vector<std::filesystem::directory_entry> Utilities::getFilesInFolder(std::filesystem::path folderPath)
{
	std::vector<std::filesystem::directory_entry> files;
	for (const auto& file : std::filesystem::directory_iterator(folderPath))
	{
		files.push_back(file);
	};
	return files;
};

std::vector<std::filesystem::directory_entry> Utilities::getFilesOfExtInFolder(std::filesystem::path folderPath, std::string ext)
{
	std::vector<std::filesystem::directory_entry> files;
	for (const auto& file : std::filesystem::directory_iterator(folderPath))
	{
		if (file.path().extension() == ext)
		{
			files.push_back(file);
		};
	};
	return files;
};


using std::string, std::vector, std::filesystem::directory_entry;

string compilerPath = "../../shaders/glslc.exe";
vector<string>* Utilities::pCompiledVertShaders = new vector<string>();
vector<string>* Utilities::pCompiledFragShaders = new vector<string>();

void Utilities::compileShaders(std::filesystem::path folderPath) {
	std::wstring tCompilerPath = std::wstring(compilerPath.begin(), compilerPath.end());
	LPCWSTR wCompilerPath = tCompilerPath.c_str();

	// Get all shader files (.vert and .frag) in the folder
	vector<directory_entry> vertShaders = getFilesOfExtInFolder(folderPath, ".vert");
	vector<directory_entry> fragShaders = getFilesOfExtInFolder(folderPath, ".frag");

	// Compile shaders
	for (const auto& vertShader : vertShaders) {
		std::filesystem::path vertShaderPath = vertShader.path();
		string vertShaderName = vertShaderPath.filename().stem().string();
		string outputName = vertShaderPath.relative_path().replace_extension(".spv").string();
		string command = vertShaderPath.string() + " -o " + outputName;

		getInstance()->iDebugPrint("Compiling vertex shader: " + vertShaderName + " | Command: " + command, "Utilities");

		ShellExecute(NULL, NULL, wCompilerPath, std::wstring(command.begin(), command.end()).c_str(), NULL, SW_HIDE);

		Utilities::pCompiledVertShaders->push_back(outputName);
	};

	for (const auto& fragShader : fragShaders) {
		std::filesystem::path fragShaderPath = fragShader.path();
		string fragShaderName = fragShaderPath.filename().stem().string();
		string outputName = fragShaderPath.relative_path().replace_extension(".spv").string();
		string command = fragShaderPath.string() + " -o " + outputName;

		getInstance()->iDebugPrint("Compiling fragment shader: " + fragShaderName + " | Command: " + command, "Utilities");

		ShellExecute(NULL, NULL, wCompilerPath, std::wstring(command.begin(), command.end()).c_str(), NULL, SW_HIDE);

		Utilities::pCompiledFragShaders->push_back(outputName);
	};
}
#include "Utilities.h"

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

string compilerPath = "\\shaders\\glslc.exe";
string compileExecPath = "\\shaders";
string Utilities::EngineWorkingDirectory = "";
vector<string>* Utilities::pCompiledVertShaders = new vector<string>();
vector<string>* Utilities::pCompiledFragShaders = new vector<string>();

void Utilities::compileShaders(std::filesystem::path folderPath) {
	string workingCompilerPath = EngineWorkingDirectory + compilerPath;

	// Get all shader files (.vert and .frag) in the folder
	vector<directory_entry> vertShaders = getFilesOfExtInFolder(folderPath, ".vert");
	vector<directory_entry> fragShaders = getFilesOfExtInFolder(folderPath, ".frag");

	// Compile shaders
	for (const auto& vertShader : vertShaders) {
		std::filesystem::path vertShaderPath = vertShader.path();
		string vertShaderName = vertShaderPath.filename().stem().string();
		std::filesystem::path outputPath = vertShaderPath;
		string outputPathS = EngineWorkingDirectory + "/" + outputPath.replace_extension(".spv").string();
		string command = workingCompilerPath + " " + EngineWorkingDirectory + "/" + vertShaderPath.string() + " -o " + outputPathS;

		getInstance()->iDebugPrint("Compiling vertex shader: " + vertShaderName + " | Command: " + command, "Utilities");

		int result = system(command.c_str());
		if (result != 0) {
			getInstance()->iDebugPrint("Failed to compile vertex shader: " + vertShaderName, "Utilities");
		}
		else {
			Utilities::pCompiledVertShaders->push_back(outputPathS);
		}
	};

	for (const auto& fragShader : fragShaders) {
		std::filesystem::path fragShaderPath = fragShader.path();
		string fragShaderName = fragShaderPath.filename().stem().string();
		std::filesystem::path outputPath = fragShaderPath;
		string outputPathS = EngineWorkingDirectory + "/" + outputPath.replace_extension(".spv").string();
		string command = workingCompilerPath + " " + EngineWorkingDirectory + "/" + fragShaderPath.string() + " -o " + outputPathS;

		getInstance()->iDebugPrint("Compiling fragment shader: " + fragShaderName + " | Command: " + command, "Utilities");

		int result = system(command.c_str());
		if (result != 0) {
			getInstance()->iDebugPrint("Failed to compile fragment shader: " + fragShaderName, "Utilities");
		}
		else {
			Utilities::pCompiledFragShaders->push_back(outputPathS);
		}
	};

	vector<directory_entry> compiledShaders = getFilesOfExtInFolder(folderPath, ".spv");
	for (const auto& compiledShader : compiledShaders) {
		getInstance()->iDebugPrint("Compiled shader: " + compiledShader.path().filename().string(), "Utilities");
	};
}
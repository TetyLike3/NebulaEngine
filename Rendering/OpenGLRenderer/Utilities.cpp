#include "Utilities.h"


using std::string, std::vector, fs::directory_entry, fs::directory_iterator, fs::path;

string Utilities::m_lastFilePrinted = "";
string Utilities::m_lastMessagePrinted = "";


string Utilities::generateTimestamp_HH_MM_SS_mmm()
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


std::string Utilities::ANSI24RGB(int R, int G, int B)
{
	return std::format("\033[38;2;{};{};{}m",R,G,B);
};

// Needs a NUL terminator, otherwise operations like shader compilation will find junk characters at the end of the file
const char* fileTerminator = "\0";

vector<char> Utilities::readFile(const string& filename)
{
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		throw std::runtime_error("failed to open file!");
	};

	size_t fileSize = (size_t)file.tellg();
	vector<char> buffer(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);
	file.close();
	buffer.insert(buffer.end(), *fileTerminator);

	return buffer;
};

vector<directory_entry> Utilities::getFilesInFolder(path folderPath)
{
	vector<directory_entry> files;
	for (const auto& file : directory_iterator(folderPath))
	{
		files.push_back(file);
	};
	return files;
};

vector<directory_entry> Utilities::getFilesOfExtInFolder(path folderPath, string ext)
{
	vector<directory_entry> files;
	for (const auto& file : directory_iterator(folderPath))
	{
		if (file.path().extension() == ext)
		{
			files.push_back(file);
		};
	};
	return files;
}
directory_entry Utilities::getFirstFileOfExtInFolder(path folderPath, string ext)
{
	for (const auto& file : directory_iterator(folderPath))
	{
		if (file.path().extension() == ext)
		{
			return file;
		};
	};
	return directory_entry();
};

vector<directory_entry> Utilities::getFoldersInFolder(path folderPath)
{
	vector<directory_entry> folders;
	for (const auto& folder : fs::recursive_directory_iterator(folderPath))
	{
		if (!folder.is_directory()) continue;
		folders.push_back(folder);
	};
	return folders;
};

string Utilities::calculateFPS(double deltaTime, int precision) {
	double fps = 1 / deltaTime;
	if (precision < 0) return std::to_string(fps);
	if (precision == 0) return std::to_string(trunc(fps));

	string fpsString = std::to_string(fps);
	return fpsString.substr(0, fpsString.find(".") + precision + 1);
}



void Utilities::iDebugPrint(string message, string fileName, string funcName, int lineNumber) {
	using std::vformat, std::make_format_args, std::cerr;

	string timestamp = generateTimestamp_HH_MM_SS_mmm();
	string separator = ANSI24RGB(255, 255, 255) + " - ";

	string relativeFileName = fileName.substr(fs::current_path().string().length());
	// Print class name only once
	if (m_lastFilePrinted != relativeFileName)
	{
		m_lastFilePrinted = relativeFileName;
		cerr << vformat("OpenGLRenderer - " + ANSI24RGB(64, 255, 64) + "{}" + separator + "\n", make_format_args(m_lastFilePrinted));
	};

	m_lastMessagePrinted = vformat(ANSI24RGB(64, 255, 255) + "[{}]" + separator + ANSI24RGB(255, 64, 64) + "DEBUG " + ANSI24RGB(255, 210, 64) + "{}" + separator + "{}\n", make_format_args(timestamp, funcName, message));
	cerr << m_lastMessagePrinted;
}
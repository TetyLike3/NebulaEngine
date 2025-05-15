#pragma once

#include <string>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <chrono>
#include <iterator>
#include <format>

namespace fs = std::filesystem;

#define mDebugPrint(x) Utilities::debugPrint(x,__FILE__, __func__, __LINE__)


class Utilities
{
public:
	template <typename FILENAME, typename FUNCNAME>
	// Prints a debug message with the class name and timestamp.
	static void debugPrint(std::string message, FILENAME fileName, FUNCNAME funcName, int lineNumber) { Utilities::iDebugPrint(message, fileName, funcName, lineNumber); };

	// Generates a timestamp in the format HH:MM:SS.mmm
	static std::string generateTimestamp_HH_MM_SS_mmm();
	static std::string ANSI24RGB(int R, int G, int B);

	static std::string EngineWorkingDirectory;
	static std::vector<char> readFile(const std::string& filename);

	static std::vector<fs::directory_entry> getFilesInFolder(fs::path folderPath);
	static std::vector<fs::directory_entry> getFilesOfExtInFolder(fs::path folderPath, std::string ext);
	static fs::directory_entry getFirstFileOfExtInFolder(fs::path folderPath, std::string ext);
	static std::vector<fs::directory_entry> getFoldersInFolder(fs::path folderPath);

	static std::string calculateFPS(double deltaTime, int precision = -1);
private:
	static std::string m_lastFilePrinted;
	static std::string m_lastMessagePrinted;
	static void iDebugPrint(std::string message, std::string fileName, std::string funcName, int lineNumber);
};
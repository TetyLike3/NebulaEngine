#pragma once

#ifndef BASESHADER_H
#define BASESHADER_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include "../Utilities.h"

using std::string;

class BaseShader
{
public:
	BaseShader(string shaderSourcePath)
	{
		using std::vector, std::filesystem::directory_entry;
		m_sourcePath = shaderSourcePath;
		mDebugPrint(m_sourcePath);

		// Get all shader files (.vert and .frag) in the folder
		directory_entry vertShader = Utilities::getFirstFileOfExtInFolder(m_sourcePath, ".vert");
		directory_entry fragShader = Utilities::getFirstFileOfExtInFolder(m_sourcePath, ".frag");

		if (!(vertShader.exists() && fragShader.exists())) {
			mDebugPrint("Skipping shader creation, missing source files");
			return;
		}

		string vertPath = vertShader.path().string();
		string fragPath = fragShader.path().string();

		vector<char> vertSource = Utilities::readFile(vertPath);
		vector<char> fragSource = Utilities::readFile(fragPath);

		const char* pVertSource = vertSource.data();
		const char* pFragSource = fragSource.data();

		unsigned int vert, frag;
		vert = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vert, 1, &pVertSource, NULL);
		glCompileShader(vert);
		checkLogErrors(vert, "VERTEX");
		mDebugPrint("Compiled vertex shader");

		frag = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(frag, 1, &pFragSource, NULL);
		glCompileShader(frag);
		checkLogErrors(frag, "FRAGMENT");
		mDebugPrint("Compiled fragment shader");

		m_ID = glCreateProgram();
		glAttachShader(m_ID, vert);
		glAttachShader(m_ID, frag);
		glLinkProgram(m_ID);
		checkLogErrors(m_ID, "PROGRAM");
		mDebugPrint("Linked shader program");

		glDeleteShader(vert);
		glDeleteShader(frag);
	};

	unsigned int m_ID = 0;
	string m_sourcePath;


	void activate() {
		glUseProgram(m_ID);
	}


	// Uniform update functions

	void setUniformBool(const string& name, bool value) const {
		glUniform1i(glGetUniformLocation(m_ID, name.c_str()), (int)value);
	}
	void setUniformInt(const string& name, int value) const {
		glUniform1i(glGetUniformLocation(m_ID, name.c_str()), value);
	}
	void setUniformFloat(const string& name, float value) const {
		glUniform1f(glGetUniformLocation(m_ID, name.c_str()), value);
	}
	void setUniformVec2(const string& name, const glm::vec2 &value) const {
		glUniform2fv(glGetUniformLocation(m_ID, name.c_str()), 1, &value[0]);
	}
	void setUniformVec2(const string& name, float x, float y) const {
		glUniform2f(glGetUniformLocation(m_ID, name.c_str()), x, y);
	}
	void setUniformVec3(const string& name, const glm::vec3& value) const {
		glUniform3fv(glGetUniformLocation(m_ID, name.c_str()), 1, &value[0]);
	}
	void setUniformVec3(const string& name, float x, float y, float z) const {
		glUniform3f(glGetUniformLocation(m_ID, name.c_str()), x, y, z);
	}
	void setUniformVec4(const string& name, const glm::vec4& value) const {
		glUniform4fv(glGetUniformLocation(m_ID, name.c_str()), 1, &value[0]);
	}
	void setUniformVec4(const string& name, float x, float y, float z, float w) const {
		glUniform4f(glGetUniformLocation(m_ID, name.c_str()), x, y, z, w);
	}
	void setUniformMat2(const string& name, const glm::mat2& matrix) const {
		glUniformMatrix2fv(glGetUniformLocation(m_ID, name.c_str()), 1, GL_FALSE, &matrix[0][0]);
	}
	void setUniformMat3(const string& name, const glm::mat3& matrix) const {
		glUniformMatrix3fv(glGetUniformLocation(m_ID, name.c_str()), 1, GL_FALSE, &matrix[0][0]);
	}
	void setUniformMat4(const string& name, const glm::mat4& matrix) const {
		glUniformMatrix4fv(glGetUniformLocation(m_ID, name.c_str()), 1, GL_FALSE, &matrix[0][0]);
	}

private:
	void checkLogErrors(GLuint shader, string type) {
		GLint success;
		GLchar log[1024];
		if (type != "PROGRAM") {
			glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
			if (!success) {
				glGetShaderInfoLog(shader, 1024, NULL, log);
				mDebugPrint("SHADER COMPILATION FAILED | " + type + "\n" + log + "\n--- COMPILE ERROR LOG END ---");
			}
		}
		else {
			glGetProgramiv(shader, GL_LINK_STATUS, &success);
			if (!success) {
				glGetProgramInfoLog(shader, 1024, NULL, log);
				mDebugPrint("PROGRAM LINK FAILED | " + type + "\n" + log + "\n--- LINK ERROR LOG END ---");
			}
		}
	}
};

#endif
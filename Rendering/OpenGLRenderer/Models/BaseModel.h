#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>

#include "../Utilities.h"
#include "../Graphics/BaseShader.h"
#include "../Graphics/BaseTexture.h"

class BaseModel
{
public:
	BaseModel(BaseShader shaderToUse, BaseTexture textureToUse, unsigned int modelVAO) : m_shader(shaderToUse), m_texture(textureToUse), m_VAO(modelVAO) {
		m_shader.activate();
		m_shader.setUniformInt("ourTexture", 0);
	};

	void DrawModel();
private:
	BaseShader m_shader;
	BaseTexture m_texture;
	unsigned int m_VAO;

	glm::mat4 m_modelTransform = glm::rotate(glm::mat4(1.0f), glm::radians(-55.0f), glm::vec3(1.0f, 0.0f, 0.0f));
};
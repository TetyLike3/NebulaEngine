#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "../Utilities/Utilities.h"
#include "../Graphics/Image.h"
#include "../Graphics/Vertex.h"
#include "../Graphics/Buffers.h"

class Model
{
public:
	Model(std::string modelPath, std::string texturePath) : m_modelPath(modelPath), m_texturePath(texturePath), m_pUtilities(Utilities::getInstance()) {
		createModel();
	};

	void createModel();

	glm::mat4 getTransform() { 
		glm::mat4 transform = glm::mat4(1.0f);
		transform = glm::translate(transform, m_position);
		transform = glm::rotate(transform, glm::radians(m_rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
		transform = glm::rotate(transform, glm::radians(m_rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
		transform = glm::rotate(transform, glm::radians(m_rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
		transform = glm::scale(transform, m_scale);
		return transform;
	}
	void changePosition(glm::vec3 newPos);
	void changeRotation(glm::vec3 newRot);
	void changeScale(glm::vec3 newScale);
	void cleanup();

private:
	Utilities* m_pUtilities = nullptr;
	static BufferManager* m_pBufferManager;
	friend class VulkanEngine;
	friend class CommandBuffer;
	friend class Window;

	glm::vec3 m_position;
	glm::vec3 m_rotation;
	glm::vec3 m_scale;

	std::vector<Vertex> m_vertices;
	std::vector<uint32_t> m_indices;
	std::string m_modelPath = "";
	std::string m_texturePath = "";
	Image* m_pTextureImage = nullptr;
	VertexBuffer* m_pVertexBuffer = nullptr;
	IndexBuffer* m_pIndexBuffer = nullptr;
	UniformBufferObject* m_pUniformBufferObject = nullptr;
	DescriptorSets* m_pDescriptorSets = nullptr;
};
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include <unordered_map>

#include "../VulkanRenderer.h"

#include "Model.h"

BufferManager* Model::m_pBufferManager = nullptr;

void Model::createModel() {
	mDebugPrint("Loading OBJ model from path: " + m_modelPath);
	m_pTextureImage = new Image(m_texturePath);

	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string warn, err;

	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, m_modelPath.c_str())) {
		throw std::runtime_error(warn + err);
	}

	m_position = { 0.0f, 0.0f, 0.0f };
	m_rotation = { 0.0f, 0.0f, 0.0f };
	m_scale = { 1.0f, 1.0f, 1.0f };

	std::unordered_map<Vertex, uint32_t> uniqueVertices{};

	for (const auto& shape : shapes) {
		for (const auto& index : shape.mesh.indices) {
			Vertex vertex{};

			vertex.pos = {
				attrib.vertices[3 * index.vertex_index + 0],
				attrib.vertices[3 * index.vertex_index + 1],
				attrib.vertices[3 * index.vertex_index + 2]
			};
			
			vertex.texCoord = {
				attrib.texcoords[2 * index.texcoord_index + 0],
				1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
			};

			vertex.color = {1.0f, 1.0f, 1.0f};

			if (uniqueVertices.count(vertex) == 0) {
				uniqueVertices[vertex] = static_cast<uint32_t>(m_vertices.size());
				m_vertices.push_back(vertex);
			}
			m_indices.push_back(uniqueVertices[vertex]);
		}
	}

	m_pVertexBuffer = new VertexBuffer(m_pBufferManager, m_vertices);
	m_pBufferManager->getVertexBuffers()->push_back(m_pVertexBuffer);
	m_pIndexBuffer = new IndexBuffer(m_pBufferManager, m_indices);
	m_pBufferManager->getIndexBuffers()->push_back(m_pIndexBuffer);
	m_pUniformBufferObject = new UniformBufferObject(m_pBufferManager);
	m_pBufferManager->getUniformBufferObjects()->push_back(m_pUniformBufferObject);

	m_pDescriptorSets = new DescriptorSets(m_pBufferManager);
	m_pDescriptorSets->createDescriptorPool();
	m_pDescriptorSets->createDescriptorSets(m_pTextureImage->getVkTextureImageView(), m_pTextureImage->getVkTextureSampler(), m_pUniformBufferObject);
}

void Model::changePosition(glm::vec3 newPos) {
	m_position = newPos;
}

void Model::changeRotation(glm::vec3 newRot) {
	m_rotation = newRot;
}

void Model::changeScale(glm::vec3 newScale) {
	m_scale = newScale;
}

void Model::cleanup() {
	m_pDescriptorSets->cleanup();
	delete m_pDescriptorSets;

	m_pUniformBufferObject->cleanup();
	delete m_pUniformBufferObject;

	m_pTextureImage->cleanup();
	delete m_pTextureImage;

	m_pVertexBuffer->cleanup();
	delete m_pVertexBuffer;

	m_pIndexBuffer->cleanup();
	delete m_pIndexBuffer;
}
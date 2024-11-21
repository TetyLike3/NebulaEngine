#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <array>
#include <vector>
#include <stdexcept>

#include "Vertex.h"
#include "Swapchain.h"


#define mfDebugPrint(x) m_pBufferManager->m_pUtilities->debugPrint(x,this)






//// ----------------------------------------------------- //
/// ------------------ Buffer Manager ------------------- //
// ----------------------------------------------------- //

class CommandBuffer;
class VertexBuffer;
class IndexBuffer;
class DepthBuffer;
class Framebuffer;
class UniformBufferObject;
class DescriptorSets;
class Model;


class BufferManager
{
public:
	BufferManager();

	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& pBuffer, VkDeviceMemory& pDeviceMemory);
	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
	static uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

	CommandBuffer* getCommandBuffer() { return m_pCommandBuffer; }
	std::vector<VertexBuffer*>* getVertexBuffers() { return &m_pVertexBuffers; }
	std::vector<IndexBuffer*>* getIndexBuffers() { return &m_pIndexBuffers; }
	DepthBuffer* getDepthBuffer() { return m_pDepthBuffer; }
	Framebuffer* getFramebuffer() { return m_pFramebuffer; }
	std::vector<UniformBufferObject*>* getUniformBufferObjects() { return &m_pUniformBufferObjects; }

private:
	VkDevice* m_pLogicalDevice = nullptr;
	static VkPhysicalDevice* m_pPhysicalDevice;
	VkSurfaceKHR* m_pSurface = nullptr;
	VkRenderPass* m_pRenderPass = nullptr;
	Swapchain* m_pSwapchain = nullptr;
	VkPipeline* m_pGraphicsPipeline = nullptr;
	VkPipelineLayout* m_pPipelineLayout = nullptr;
	Utilities* m_pUtilities = nullptr;
	sSettings* m_pSettings = nullptr;
	int m_MAX_FRAMES_IN_FLIGHT = 1;
	VkQueue* m_pGraphicsQueue = nullptr;
	VkDescriptorSetLayout* m_pDescriptorSetLayout = nullptr;


	CommandBuffer* m_pCommandBuffer = nullptr;
	std::vector<VertexBuffer*> m_pVertexBuffers;
	std::vector<IndexBuffer*> m_pIndexBuffers;
	std::vector<UniformBufferObject*> m_pUniformBufferObjects;
	std::vector<Model*>* m_pLoadedModels = nullptr;
	DepthBuffer* m_pDepthBuffer = nullptr;
	Framebuffer* m_pFramebuffer = nullptr;

	friend class VulkanEngine;
	friend class CommandBuffer;
	friend class VertexBuffer;
	friend class IndexBuffer;
	friend class DepthBuffer;
	friend class Framebuffer;
	friend class UniformBufferObject;
	friend class DescriptorSets;
};



//// ------------------------------------------------------- //
/// ------------------ Buffer Base class ------------------ //
// ------------------------------------------------------- //


class BaseBuffer
{
public:
	BaseBuffer(BufferManager* pBufferManager) : m_pBufferManager(pBufferManager) {};

	//virtual void createBuffer() = 0;
	virtual void cleanup() = 0;

protected:
	BufferManager* m_pBufferManager = nullptr;
};




//// ---------------------------------------------------- //
/// ------------------ Command Buffer ------------------ //
// ---------------------------------------------------- //


class CommandBuffer
{
public:
	/*
	CommandBuffer(VkDevice* pLogicalDevice, VkPhysicalDevice* pPhysicalDevice, VkSurfaceKHR* pSurface, VkRenderPass* pRenderPass, Swapchain* pSwapchain, VkPipeline* pGraphicsPipeline)
		: m_pLogicalDevice(pLogicalDevice), m_pPhysicalDevice(pPhysicalDevice), m_pSurface(pSurface), m_pRenderPass(pRenderPass), m_pSwapchain(pSwapchain), m_pGraphicsPipeline(pGraphicsPipeline),
		m_pUtilities(Utilities::getInstance())
	{
		createCommandPool();
	};
	*/
	CommandBuffer(BufferManager* pBufferManager) : m_pBufferManager(pBufferManager)
	{
		createCommandPool();
	};

	void createCommandPool();
	VkCommandBuffer beginSingleTimeCommands();
	void endSingleTimeCommands(VkCommandBuffer commandBuffer);
	void createCommandBuffers();
	void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

	void cleanup();

	VkCommandPool* getVkCommandPool() { return &sm_commandPool; }
	std::vector<VkCommandBuffer>* getCommandBuffers() { return &sm_commandBuffers; }

private:
	BufferManager* m_pBufferManager = nullptr;

	static VkCommandPool sm_commandPool;
	static std::vector<VkCommandBuffer> sm_commandBuffers;
};





//// ----------------------------------------------------- //
/// ------------------- Vertex Buffer ------------------- //
// ----------------------------------------------------- //


class VertexBuffer
{
public:
	std::vector<Vertex> m_vertices;

	VertexBuffer(BufferManager* pBufferManager, std::vector<Vertex> vertices) : m_pBufferManager(pBufferManager), m_vertices(vertices)
	{
		createVertexBuffer();
	};


	void createVertexBuffer();

	void cleanup();

	void recreateVertexBuffer(std::vector<Vertex> vertices);

	VkBuffer* getVkVertexBuffer() { return &m_vertexBuffer; }

private:
	BufferManager* m_pBufferManager = nullptr;

	VkBuffer m_vertexBuffer = VK_NULL_HANDLE;
	VkDeviceMemory m_vertexBufferMemory = VK_NULL_HANDLE;
};








//// ----------------------------------------------------- //
/// ------------------- Index Buffer -------------------- //
// ----------------------------------------------------- //


class IndexBuffer
{
public:
	std::vector<uint32_t> m_indices;

	IndexBuffer(BufferManager* pBufferManager, std::vector<uint32_t> indices) : m_pBufferManager(pBufferManager), m_indices(indices)
	{
		createIndexBuffer();
	};

	void createIndexBuffer();

	void cleanup();

	void recreateIndexBuffer(std::vector<uint32_t> indices);

	VkBuffer* getVkIndexBuffer() { return &m_indexBuffer; }

private:
	BufferManager* m_pBufferManager = nullptr;

	VkBuffer m_indexBuffer = VK_NULL_HANDLE;
	VkDeviceMemory m_indexBufferMemory = VK_NULL_HANDLE;
};







//// ----------------------------------------------------- //
/// -------------------- Depth Buffer ------------------- //
// ----------------------------------------------------- //

class DepthBuffer
{
public:
	DepthBuffer(BufferManager* pBufferManager) : m_pBufferManager(pBufferManager)
	{
		createDepthResources();
	};

	void createDepthResources();
	static VkFormat findDepthFormat(VkPhysicalDevice* pPhysicalDevice);
	static VkFormat findSupportedFormat(VkPhysicalDevice* pPhysicalDevice, const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

	void cleanup();

	VkImageView* getVkImageView() { return &m_depthImageView; }
private:
	BufferManager* m_pBufferManager = nullptr;

	VkImage m_depthImage = VK_NULL_HANDLE;
	VkDeviceMemory m_depthImageMemory = VK_NULL_HANDLE;
	VkImageView m_depthImageView = VK_NULL_HANDLE;
};






//// ----------------------------------------------------- //
/// -------------------- Frame Buffer ------------------- //
// ----------------------------------------------------- //


class Framebuffer
{
public:
	Framebuffer(BufferManager* pBufferManager) : m_pBufferManager(pBufferManager)
	{
		createFramebuffers();
	};

	void createFramebuffers();
	void cleanup();

	std::vector<VkFramebuffer>* getFramebuffers() { return &m_framebuffers; }
private:
	BufferManager* m_pBufferManager = nullptr;

	std::vector<VkFramebuffer> m_framebuffers = {};
};







//// ----------------------------------------------------- //
/// ------------------ Uniform Buffers ------------------ //
// ----------------------------------------------------- //


class UniformBufferObject
{
public:
	struct sUniformBufferObject
	{
		alignas(16) glm::mat4 model;
		alignas(16) glm::mat4 view;
		alignas(16) glm::mat4 proj;
	};

	UniformBufferObject(BufferManager* pBufferManager) : m_pBufferManager(pBufferManager)
	{
		createUniformBuffers();
	};

	void createUniformBuffers();

	void cleanup();


	std::vector<VkBuffer>* getUniformBuffers() { return &m_uniformBuffers; };
	std::vector<void*>* getUniformBuffersMapped() { return &m_uniformBuffersMapped; };

private:
	BufferManager* m_pBufferManager = nullptr;


	std::vector<VkBuffer> m_uniformBuffers = {};
	std::vector<VkDeviceMemory> m_uniformBuffersMemory = {};
	std::vector<void*> m_uniformBuffersMapped = {};
};







//// ----------------------------------------------------- //
/// ------------------ Descriptor Sets ------------------ //
// ----------------------------------------------------- //


class DescriptorSets
{
public:
	DescriptorSets(BufferManager* pBufferManager) : m_pBufferManager(pBufferManager)
	{
		createDescriptorPool();
		//createDescriptorSets(); // Called in VulkanEngine to get the image views and samplers
	};

	void createDescriptorPool();
	void createDescriptorSets(VkImageView* pImageView, VkSampler* pImageSampler, UniformBufferObject* pUniformBuffer);

	void cleanup();

	std::vector<VkDescriptorSet>* getVkDescriptorSets() { return m_pDescriptorSets; }

private:
	BufferManager* m_pBufferManager = nullptr;


	VkDescriptorPool* m_pDescriptorPool = nullptr;
	std::vector<VkDescriptorSet>* m_pDescriptorSets = nullptr;
};
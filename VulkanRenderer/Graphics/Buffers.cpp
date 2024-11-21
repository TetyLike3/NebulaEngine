#include "../VulkanRenderer.h"
#include "Image.h"
#include "Swapchain.h"
#include "GraphicsPipeline.h"

#include "Buffers.h"


//// ----------------------------------------------------- //
/// ------------------ Buffer Manager ------------------- //
// ----------------------------------------------------- //


VkPhysicalDevice* BufferManager::m_pPhysicalDevice = nullptr;

BufferManager::BufferManager() : m_pLogicalDevice(VulkanEngine::getInstance()->m_pLogicalDevice->getVkDevice()), m_pSurface(VulkanEngine::getInstance()->m_pVkSurface),
m_pRenderPass(VulkanEngine::getInstance()->m_pGraphicsPipeline->getRenderPass()), m_pSwapchain(VulkanEngine::getInstance()->m_pSwapchain), m_pSettings(VulkanEngine::getInstance()->m_settings),
m_MAX_FRAMES_IN_FLIGHT(VulkanEngine::getInstance()->m_MAX_FRAMES_IN_FLIGHT), m_pGraphicsPipeline(VulkanEngine::getInstance()->m_pGraphicsPipeline->getGraphicsPipeline()),
m_pGraphicsQueue(VulkanEngine::getInstance()->m_pLogicalDevice->getGraphicsQueue()), m_pDescriptorSetLayout(VulkanEngine::getInstance()->m_pGraphicsPipeline->getDescriptorSetLayout()),
m_pPipelineLayout(VulkanEngine::getInstance()->m_pGraphicsPipeline->getVkPipelineLayout()), m_pUtilities(Utilities::getInstance())
{
	if (m_pPhysicalDevice == nullptr)
		m_pPhysicalDevice = VulkanEngine::getInstance()->m_pPhysicalDevice->getVkPhysicalDevice();
};

void BufferManager::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& pBuffer, VkDeviceMemory& pBufferMemory)
{
	VkBufferCreateInfo bufferInfo{
	.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
	.size = size,
	.usage = usage,
	.sharingMode = VK_SHARING_MODE_EXCLUSIVE
	};

	if (vkCreateBuffer(*m_pLogicalDevice, &bufferInfo, nullptr, &pBuffer) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create buffer.");
	}

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(*m_pLogicalDevice, pBuffer, &memRequirements);

	VkMemoryAllocateInfo allocInfo{
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.allocationSize = memRequirements.size,
		.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties)
	};

	if (vkAllocateMemory(*m_pLogicalDevice, &allocInfo, nullptr, &pBufferMemory) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to allocate vertex buffer memory.");
	}

	vkBindBufferMemory(*m_pLogicalDevice, pBuffer, pBufferMemory, 0);
};

void BufferManager::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
	VkCommandBuffer commandBuffer = m_pCommandBuffer->beginSingleTimeCommands();

	VkBufferCopy copyRegion{
		.size = size
	};

	vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

	m_pCommandBuffer->endSingleTimeCommands(commandBuffer);
}

uint32_t BufferManager::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(*m_pPhysicalDevice, &memProperties);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
		if (typeFilter & (1 << i) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}

	throw std::runtime_error("failed to find suitable memory type!");
}




//// ----------------------------------------------------- //
/// ------------------ Command Bufffer ------------------ //
// ----------------------------------------------------- //

VkCommandPool CommandBuffer::sm_commandPool = VK_NULL_HANDLE;
std::vector<VkCommandBuffer> CommandBuffer::sm_commandBuffers = {};

void CommandBuffer::createCommandPool()
{
	mfDebugPrint("Creating command pool...");

	QueueFamilyIndices::sQueueFamilyIndices queueFamilyIndices = QueueFamilyIndices::findQueueFamilies(*m_pBufferManager->m_pPhysicalDevice, *m_pBufferManager->m_pSurface);

	VkCommandPoolCreateInfo poolInfo{
		.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		.pNext = nullptr,
		.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
		.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value()
	};

	if (vkCreateCommandPool(*m_pBufferManager->m_pLogicalDevice, &poolInfo, nullptr, &sm_commandPool) != VK_SUCCESS) {
		throw std::runtime_error("failed to create command pool!");
	}
}

VkCommandBuffer CommandBuffer::beginSingleTimeCommands()
{
	VkCommandBufferAllocateInfo allocInfo{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.commandPool = sm_commandPool,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = 1
	};

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(*m_pBufferManager->m_pLogicalDevice, &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
	};

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	return commandBuffer;
}

void CommandBuffer::endSingleTimeCommands(VkCommandBuffer commandBuffer)
{
	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo{
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.commandBufferCount = 1,
		.pCommandBuffers = &commandBuffer
	};

	vkQueueSubmit(*m_pBufferManager->m_pGraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(*m_pBufferManager->m_pGraphicsQueue);

	vkFreeCommandBuffers(*m_pBufferManager->m_pLogicalDevice, sm_commandPool, 1, &commandBuffer);
}

void CommandBuffer::createCommandBuffers()
{
	mfDebugPrint("Creating command buffers...");

	sm_commandBuffers.resize(m_pBufferManager->m_MAX_FRAMES_IN_FLIGHT);

	VkCommandBufferAllocateInfo allocInfo{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.commandPool = sm_commandPool,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = (uint32_t)sm_commandBuffers.size()
	};

	if (vkAllocateCommandBuffers(*m_pBufferManager->m_pLogicalDevice, &allocInfo, sm_commandBuffers.data()) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate command buffers!");
	}
}

void CommandBuffer::recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
{
	VkCommandBufferBeginInfo beginInfo{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.pNext = nullptr,
		.flags = 0, // Optional
		.pInheritanceInfo = nullptr // Optional
	};

	if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
		throw std::runtime_error("failed to begin recording command buffer!");
	}

	std::vector<VkFramebuffer> framebuffers = *m_pBufferManager->m_pFramebuffer->getFramebuffers();
	std::array<VkClearValue, 2> clearValues{
		VkClearValue{{{0.1f, 0.1f, 0.1f, 1.0f}}},
		VkClearValue{{{1.0f, 0}}}
	};

	VkRenderPassBeginInfo renderPassInfo{
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
		.renderPass = *m_pBufferManager->m_pRenderPass,
		.framebuffer = framebuffers[imageIndex],
		.renderArea {
			.offset = { 0, 0 },
			.extent = *m_pBufferManager->m_pSwapchain->getSwapchainExtent(),
		},
		.clearValueCount = static_cast<uint32_t>(clearValues.size()),
		.pClearValues = clearValues.data()
	};

	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);


	VkViewport viewport{
		.x = 0.0f,
		.y = 0.0f,
		.width = static_cast<float>(m_pBufferManager->m_pSwapchain->getSwapchainExtent()->width),
		.height = static_cast<float>(m_pBufferManager->m_pSwapchain->getSwapchainExtent()->height),
		.minDepth = 0.0f,
		.maxDepth = 1.0f
	};
	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

	VkRect2D scissor{
		.offset = { 0, 0 },
		.extent = *m_pBufferManager->m_pSwapchain->getSwapchainExtent()
	};
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

	

	
	for (int i = 0; i < m_pBufferManager->m_pLoadedModels->size(); i++) {
		Model* model = m_pBufferManager->m_pLoadedModels->at(i);
		VkDeviceSize offsets[] = { 0 };

		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *m_pBufferManager->m_pGraphicsPipeline);

		vkCmdBindVertexBuffers(commandBuffer, 0, 1, model->m_pVertexBuffer->getVkVertexBuffer(), offsets);
		vkCmdBindIndexBuffer(commandBuffer, *model->m_pIndexBuffer->getVkIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *m_pBufferManager->m_pPipelineLayout, 0, 1, &(*model->m_pDescriptorSets->getVkDescriptorSets())[imageIndex], 0, nullptr);
		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(model->m_pIndexBuffer->m_indices.size()), 1, 0, 0, 0);
	}

	vkCmdEndRenderPass(commandBuffer);

	if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
		throw std::runtime_error("failed to record command buffer!");
	}
}


void CommandBuffer::cleanup()
{
	vkDestroyCommandPool(*m_pBufferManager->m_pLogicalDevice, sm_commandPool, nullptr);
}








//// ----------------------------------------------------- //
/// ------------------- Vertex Buffer ------------------- //
// ----------------------------------------------------- //


/*
std::vector<Vertex> VertexBuffer::vertices = {
	{{-1.0f, -1.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 0.0f}, 0.0f},
	{{1.0f, -1.0f, 0.0f}, {0.2f, 0.0f, 0.8f}, {0.0f, 0.0f}, 0.0f},
	{{1.0f, 1.0f, 0.0f}, {0.2f, 0.5f, 0.8f}, {0.0f, 1.0f}, 0.0f},
	{{-1.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}, 0.0f},

	{{-1.0f, 0.0f, -1.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 0.0f}, 0.0f},
	{{1.0f, 0.0f, -1.0f}, {0.2f, 0.0f, 0.8f}, {0.0f, 0.0f}, 0.0f},
	{{1.0f, 0.0f, 1.0f}, {0.2f, 0.5f, 0.8f}, {0.0f, 1.0f}, 0.0f},
	{{-1.0f, 0.0f, -1.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}, 0.0f},

	{{-0.5f, 0.5f, -1.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 0.0f}, 0.0f},
	{{0.5f, 0.5f, -1.0f}, {0.2f, 0.0f, 0.8f}, {0.0f, 0.0f}, 0.0f},
	{{0.5f, 1.5f, -1.0f}, {0.2f, 0.5f, 0.8f}, {0.0f, 1.0f}, 0.0f},
	{{-0.5f, 1.5f, -1.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}, 0.0f}
};

const std::vector<uint32_t> VertexBuffer::indices = {
	0, 1, 2, 2, 3, 0,
	4, 5, 6, 6, 7, 4,
	8, 9, 10, 10, 11, 8
};
*/


void VertexBuffer::createVertexBuffer()
{
	mfDebugPrint("Creating vertex buffer...");

	VkDeviceSize bufferSize = sizeof(m_vertices[0]) * m_vertices.size();

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	m_pBufferManager->createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	void* data;
	vkMapMemory(*m_pBufferManager->m_pLogicalDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, m_vertices.data(), (size_t)bufferSize);
	vkUnmapMemory(*m_pBufferManager->m_pLogicalDevice, stagingBufferMemory);

	m_pBufferManager->createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_vertexBuffer, m_vertexBufferMemory);

	m_pBufferManager->copyBuffer(stagingBuffer, m_vertexBuffer, bufferSize);

	vkDestroyBuffer(*m_pBufferManager->m_pLogicalDevice, stagingBuffer, nullptr);
	vkFreeMemory(*m_pBufferManager->m_pLogicalDevice, stagingBufferMemory, nullptr);
}

void VertexBuffer::cleanup()
{

	vkDestroyBuffer(*m_pBufferManager->m_pLogicalDevice, m_vertexBuffer, nullptr);
	vkFreeMemory(*m_pBufferManager->m_pLogicalDevice, m_vertexBufferMemory, nullptr);
}

void VertexBuffer::recreateVertexBuffer(std::vector<Vertex> vertices)
{
	m_vertices = vertices;
	cleanup();
	createVertexBuffer();
}









//// ----------------------------------------------------- //
/// ------------------- Index Buffer -------------------- //
// ----------------------------------------------------- //

void IndexBuffer::createIndexBuffer()
{
	mfDebugPrint("Creating index buffer...");
	VkDeviceSize bufferSize = sizeof(m_indices[0]) * m_indices.size();

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	m_pBufferManager->createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	void* data;
	vkMapMemory(*m_pBufferManager->m_pLogicalDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, m_indices.data(), (size_t)bufferSize);
	vkUnmapMemory(*m_pBufferManager->m_pLogicalDevice, stagingBufferMemory);

	m_pBufferManager->createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_indexBuffer, m_indexBufferMemory);

	m_pBufferManager->copyBuffer(stagingBuffer, m_indexBuffer, bufferSize);

	vkDestroyBuffer(*m_pBufferManager->m_pLogicalDevice, stagingBuffer, nullptr);
	vkFreeMemory(*m_pBufferManager->m_pLogicalDevice, stagingBufferMemory, nullptr);
}

void IndexBuffer::cleanup()
{
	vkDestroyBuffer(*m_pBufferManager->m_pLogicalDevice, m_indexBuffer, nullptr);
	vkFreeMemory(*m_pBufferManager->m_pLogicalDevice, m_indexBufferMemory, nullptr);
}

void IndexBuffer::recreateIndexBuffer(std::vector<uint32_t> indices)
{
	m_indices = indices;
	cleanup();
	createIndexBuffer();
}









//// ----------------------------------------------------- //
/// -------------------- Depth Buffer ------------------- //
// ----------------------------------------------------- //

void DepthBuffer::createDepthResources()
{
	VkExtent2D swapchainExtent = *m_pBufferManager->m_pSwapchain->getSwapchainExtent();

	VkFormat depthFormat = findDepthFormat(m_pBufferManager->m_pPhysicalDevice);
	Image::createImage(swapchainExtent.width, swapchainExtent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_depthImage, m_depthImageMemory);

	m_depthImageView = Image::createImageView(m_depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
	Image::transitionImageLayout(m_depthImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
}

VkFormat DepthBuffer::findDepthFormat(VkPhysicalDevice* pPhysicalDevice)
{
	return findSupportedFormat(pPhysicalDevice,
		{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
		VK_IMAGE_TILING_OPTIMAL,
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
	);
}

VkFormat DepthBuffer::findSupportedFormat(VkPhysicalDevice* pPhysicalDevice, const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
{
	for (VkFormat format : candidates)
	{
		VkFormatProperties props;
		vkGetPhysicalDeviceFormatProperties(*pPhysicalDevice, format, &props);

		if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
			return(format);
		else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
			return(format);

	}

	throw std::runtime_error("Failed to find supported format!");
}



void DepthBuffer::cleanup()
{
	vkDestroyImageView(*m_pBufferManager->m_pLogicalDevice, m_depthImageView, nullptr);
	vkDestroyImage(*m_pBufferManager->m_pLogicalDevice, m_depthImage, nullptr);
	vkFreeMemory(*m_pBufferManager->m_pLogicalDevice, m_depthImageMemory, nullptr);
}






//// ----------------------------------------------------- //
/// -------------------- Frame Buffer ------------------- //
// ----------------------------------------------------- //

void Framebuffer::createFramebuffers()
{
	Swapchain* pSwapchain = m_pBufferManager->m_pSwapchain;
	std::vector<VkImageView> swapchainImageViews = *pSwapchain->getSwapchainImageViews();
	VkExtent2D swapchainExtent = *pSwapchain->getSwapchainExtent();

	m_framebuffers.resize(swapchainImageViews.size());

	for (size_t i = 0; i < swapchainImageViews.size(); i++)
	{
		std::array<VkImageView, 2> attachments = {
			swapchainImageViews[i],
			*m_pBufferManager->m_pDepthBuffer->getVkImageView()
		};

		VkFramebufferCreateInfo framebufferInfo{
			.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
			.renderPass = *m_pBufferManager->m_pRenderPass,
			.attachmentCount = static_cast<uint32_t>(attachments.size()),
			.pAttachments = attachments.data(),
			.width = swapchainExtent.width,
			.height = swapchainExtent.height,
			.layers = 1
		};

		if (vkCreateFramebuffer(*m_pBufferManager->m_pLogicalDevice, &framebufferInfo, nullptr, &m_framebuffers[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create framebuffer!");
		}
	}
}

void Framebuffer::cleanup()
{
	for (auto framebuffer : m_framebuffers) {
		vkDestroyFramebuffer(*m_pBufferManager->m_pLogicalDevice, framebuffer, nullptr);
	}
}








//// ----------------------------------------------------- //
/// ------------------ Uniform Buffers ------------------ //
// ----------------------------------------------------- //


void UniformBufferObject::createUniformBuffers()
{
	mfDebugPrint("Creating uniform buffers...");

	VkDeviceSize bufferSize = sizeof(sUniformBufferObject);

	m_uniformBuffers.resize(m_pBufferManager->m_MAX_FRAMES_IN_FLIGHT);
	m_uniformBuffersMemory.resize(m_pBufferManager->m_MAX_FRAMES_IN_FLIGHT);
	m_uniformBuffersMapped.resize(m_pBufferManager->m_MAX_FRAMES_IN_FLIGHT);

	for (size_t i = 0; i < m_pBufferManager->m_MAX_FRAMES_IN_FLIGHT; i++) {
		m_pBufferManager->createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_uniformBuffers[i], m_uniformBuffersMemory[i]);

		vkMapMemory(*m_pBufferManager->m_pLogicalDevice, m_uniformBuffersMemory[i], 0, bufferSize, 0, &m_uniformBuffersMapped[i]);
	}

	m_uniformBuffersMapped.resize(m_pBufferManager->m_MAX_FRAMES_IN_FLIGHT);
}

void UniformBufferObject::cleanup()
{
	for (size_t i = 0; i < m_pBufferManager->m_MAX_FRAMES_IN_FLIGHT; i++) {
		vkDestroyBuffer(*m_pBufferManager->m_pLogicalDevice, m_uniformBuffers[i], nullptr);
		vkFreeMemory(*m_pBufferManager->m_pLogicalDevice, m_uniformBuffersMemory[i], nullptr);
	}
}






//// ----------------------------------------------------- //
/// ------------------ Descriptor Sets ------------------ //
// ----------------------------------------------------- //


void DescriptorSets::createDescriptorPool()
{
	mfDebugPrint("Creating descriptor pool...");

	std::array<VkDescriptorPoolSize, 2> poolSizes{
		VkDescriptorPoolSize{
			.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.descriptorCount = static_cast<uint32_t>(m_pBufferManager->m_MAX_FRAMES_IN_FLIGHT)
		},
			VkDescriptorPoolSize{
			.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			.descriptorCount = static_cast<uint32_t>(m_pBufferManager->m_MAX_FRAMES_IN_FLIGHT)
		},
		/* Remember to change the array size when adding this
		* 
			VkDescriptorPoolSize{
			.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			.descriptorCount = static_cast<uint32_t>(m_pBufferManager->m_MAX_FRAMES_IN_FLIGHT)
		}
		*/
	};

	VkDescriptorPoolCreateInfo poolInfo{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		.maxSets = static_cast<uint32_t>(m_pBufferManager->m_MAX_FRAMES_IN_FLIGHT),
		.poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
		.pPoolSizes = poolSizes.data()
	};

	VkDescriptorPool* descriptorPool = new VkDescriptorPool();
	if (vkCreateDescriptorPool(*m_pBufferManager->m_pLogicalDevice, &poolInfo, nullptr, descriptorPool) != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor pool!");
	}

	m_pDescriptorPool = descriptorPool;
}

void DescriptorSets::createDescriptorSets(VkImageView* pImageView, VkSampler* pImageSampler, UniformBufferObject* pUniformBuffer)
{
	mfDebugPrint("Creating descriptor sets...");

	std::vector<VkDescriptorSetLayout> layouts(m_pBufferManager->m_MAX_FRAMES_IN_FLIGHT, *m_pBufferManager->m_pDescriptorSetLayout);
	VkDescriptorSetAllocateInfo allocInfo{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		.descriptorPool = *m_pDescriptorPool,
		.descriptorSetCount = static_cast<uint32_t>(m_pBufferManager->m_MAX_FRAMES_IN_FLIGHT),
		.pSetLayouts = layouts.data()
	};

	m_pDescriptorSets = new std::vector<VkDescriptorSet>;

	m_pDescriptorSets->resize(m_pBufferManager->m_MAX_FRAMES_IN_FLIGHT);
	if (vkAllocateDescriptorSets(*m_pBufferManager->m_pLogicalDevice, &allocInfo, m_pDescriptorSets->data()) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate descriptor sets!");
	}

	for (size_t i = 0; i < m_pBufferManager->m_MAX_FRAMES_IN_FLIGHT; i++)
	{
		VkDescriptorBufferInfo bufferInfo{
			.buffer = (*pUniformBuffer->getUniformBuffers())[i],
			.offset = 0,
			.range = sizeof(UniformBufferObject::sUniformBufferObject)
		};

		VkDescriptorImageInfo imageInfo{
			.sampler = *pImageSampler,
			.imageView = *pImageView,
			.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
		};

		/*
		VkDescriptorImageInfo heightmapInfo{
			.sampler = *pHeightmapSampler,
			.imageView = *pHeightmapView,
			.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
		};
		*/

		std::vector<VkDescriptorSet> descriptorSets = *m_pDescriptorSets;
		std::array<VkWriteDescriptorSet, 2> descriptorWrites{
			VkWriteDescriptorSet{
				.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
				.dstSet = descriptorSets[i],
				.dstBinding = 0,
				.dstArrayElement = 0,
				.descriptorCount = 1,
				.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
				.pBufferInfo = &bufferInfo
			},
				VkWriteDescriptorSet{
				.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
				.dstSet = descriptorSets[i],
				.dstBinding = 1,
				.dstArrayElement = 0,
				.descriptorCount = 1,
				.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
				.pImageInfo = &imageInfo
			}
		};

		vkUpdateDescriptorSets(*m_pBufferManager->m_pLogicalDevice, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
	}
}

void DescriptorSets::cleanup()
{
	m_pBufferManager->mDebugPrint("cleaning up descriptor pool");
	vkDestroyDescriptorPool(*m_pBufferManager->m_pLogicalDevice, *m_pDescriptorPool, nullptr);
	delete m_pDescriptorPool;
}
#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>

#include "../Utilities/Utilities.h"
#include "Devices.h"
#include "Buffers.h"



class VulkanEngine;

class Image
{
public:
	Image(std::string imagePath) : m_imagePath(imagePath), m_pUtilities(Utilities::getInstance())
	{
		createTextureImage();
		createTextureImageView();
		createTextureSampler();
	};

	void createTextureImage();
	void createTextureImageView();
	void createTextureSampler();

	static VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
	static void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
	static bool hasStencilComponent(VkFormat format);
	static void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
	void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

	void cleanup();

	VkImageView* getVkTextureImageView() { return &m_textureImageView; };
	VkSampler* getVkTextureSampler() { return &m_textureSampler; };

private:
	Utilities* m_pUtilities = nullptr;
	static VkDevice* m_pLogicalDevice;
	static BufferManager* m_pBufferManager;
	static sSettings::sGraphicsSettings* m_pGraphicsSettings;


	std::string m_imagePath = "";
	VkImage m_textureImage = VK_NULL_HANDLE;
	VkDeviceMemory m_textureImageMemory = VK_NULL_HANDLE;
	VkImageView m_textureImageView = VK_NULL_HANDLE;
	VkSampler m_textureSampler = VK_NULL_HANDLE;

	friend class VulkanEngine;
};


#include "../VulkanRenderer.h"
#include "Buffers.h"

#include "GraphicsPipeline.h"


GraphicsPipeline::GraphicsPipeline() : m_pLogicalDevice(VulkanEngine::getInstance()->m_pLogicalDevice->getVkDevice()), m_pPhysicalDevice(VulkanEngine::getInstance()->m_pPhysicalDevice->getVkPhysicalDevice()),
m_pSwapchain(VulkanEngine::getInstance()->m_pSwapchain), m_pGraphicsSettings(&VulkanEngine::getInstance()->m_settings->graphicsSettings), m_pUtilities(Utilities::getInstance())
{
	m_msaaSamples = VulkanEngine::getInstance()->m_pPhysicalDevice->getMaxUsableSampleCount();

	createRenderPass();
	createDescriptorSetLayout();
	createGraphicsPipeline();
};

void GraphicsPipeline::createRenderPass()
{
	mDebugPrint("Creating render pass...");

	// Attachment setup
	VkAttachmentDescription colorAttachment{
		.format = *m_pSwapchain->getSwapchainImageFormat(),
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
		.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
		.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
	};

	VkAttachmentDescription depthAttachment{
		.format = DepthBuffer::findDepthFormat(m_pPhysicalDevice),
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
		.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE, // We don't need the depth buffer after drawing has finished
		.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
	};

	// Attachment reference
	VkAttachmentReference colorAttachmentRef{
		.attachment = 0,
		.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
	};

	VkAttachmentReference depthAttachmentRef{
		.attachment = 1,
		.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
	};

	// Subpass
	VkSubpassDescription subpass{
		.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
		.colorAttachmentCount = 1,
		.pColorAttachments = &colorAttachmentRef,
		.pDepthStencilAttachment = &depthAttachmentRef
	};

	// Subpass dependency
	VkSubpassDependency dependency{
		.srcSubpass = VK_SUBPASS_EXTERNAL,
		.dstSubpass = 0,
		.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
		.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
		.srcAccessMask = 0,
		.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT
	};

	// Render pass
	std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };
	VkRenderPassCreateInfo renderPassInfo{
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
		.attachmentCount = static_cast<uint32_t>(attachments.size()),
		.pAttachments = attachments.data(),
		.subpassCount = 1,
		.pSubpasses = &subpass,
		.dependencyCount = 1,
		.pDependencies = &dependency
	};

	if (vkCreateRenderPass(*m_pLogicalDevice, &renderPassInfo, nullptr, &m_renderPass) != VK_SUCCESS) {
		throw std::runtime_error("failed to create render pass!");
	}


}

void GraphicsPipeline::createDescriptorSetLayout()
{
	mDebugPrint("Creating uniform buffer descriptor set layout...");
	VkDescriptorSetLayoutBinding uboLayoutBinding{
		.binding = 0,
		.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		.descriptorCount = 1,
		.stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
		.pImmutableSamplers = nullptr
	};

	mDebugPrint("Creating texture sampler descriptor set layout...");
	VkDescriptorSetLayoutBinding textureSamplerLayoutBinding{
		.binding = 1,
		.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		.descriptorCount = 1,
		.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
		.pImmutableSamplers = nullptr
	};

	/*
	mDebugPrint("Creating heightmap sampler descriptor set layout...");
	VkDescriptorSetLayoutBinding heightSamplerLayoutBinding{
		.binding = 2,
		.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		.descriptorCount = 1,
		.stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
		.pImmutableSamplers = nullptr
	};
	*/

	std::array<VkDescriptorSetLayoutBinding, 2> bindings = { uboLayoutBinding, textureSamplerLayoutBinding /*, heightSamplerLayoutBinding */};

	VkDescriptorSetLayoutCreateInfo layoutInfo{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		.bindingCount = static_cast<uint32_t>(bindings.size()),
		.pBindings = bindings.data()
	};

	if (vkCreateDescriptorSetLayout(*m_pLogicalDevice, &layoutInfo, nullptr, &m_descriptorSetLayout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor set layouts!");
	}
}

void GraphicsPipeline::createGraphicsPipeline()
{
	mDebugPrint("Creating graphics pipeline layout...");

	mDebugPrint("Creating shader stages...");

	std::vector<VkPipelineShaderStageCreateInfo> shaderStagesV;
	for (int i = 0; i < Utilities::pCompiledVertShaders->size(); i++) {
		auto vertShader = Utilities::pCompiledVertShaders->at(i);

		auto vertShaderCode = m_pUtilities->readFile(vertShader);
		VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);


		VkPipelineShaderStageCreateInfo vertShaderStageInfo{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			.stage = VK_SHADER_STAGE_VERTEX_BIT,
			.module = vertShaderModule,
			.pName = "main"
		};

		shaderStagesV.push_back(vertShaderStageInfo);
	}

	for (int i = 0; i < Utilities::pCompiledFragShaders->size(); i++) {
		auto fragShader = Utilities::pCompiledFragShaders->at(i);

		auto fragShaderCode = m_pUtilities->readFile(fragShader);
		VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

		VkPipelineShaderStageCreateInfo fragShaderStageInfo{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			.stage = VK_SHADER_STAGE_FRAGMENT_BIT,
			.module = fragShaderModule,
			.pName = "main"
		};
		shaderStagesV.push_back(fragShaderStageInfo);
	}


	VkPipelineShaderStageCreateInfo* shaderStages = &shaderStagesV[0];
	auto bindingDescription = Vertex::getBindingDescription();
	auto attributeDescriptions = Vertex::getAttributeDescriptions();

	VkPipelineVertexInputStateCreateInfo vertexInputInfo{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		.vertexBindingDescriptionCount = 1,
		.pVertexBindingDescriptions = &bindingDescription,
		.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size()),
		.pVertexAttributeDescriptions = attributeDescriptions.data()
	};

	VkPipelineInputAssemblyStateCreateInfo inputAssembly{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
		.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
		.primitiveRestartEnable = VK_FALSE
	};

	mDebugPrint("Creating viewport state...");
	VkPipelineViewportStateCreateInfo viewportState{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
		.viewportCount = 1,
		.pViewports = nullptr, // Dynamic
		.scissorCount = 1,
		.pScissors = nullptr // Dynamic
	};

	mDebugPrint("Creating rasterizer...");
	//Rasterizer
	VkPipelineRasterizationStateCreateInfo rasterizer{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
		.depthClampEnable = m_pGraphicsSettings->rasterizerDepthClamp,
		.rasterizerDiscardEnable = VK_FALSE,
		//rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		//.cullMode = VK_CULL_MODE_BACK_BIT,
		.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
		.depthBiasEnable = VK_FALSE,
		.depthBiasConstantFactor = 0.0f, // Optional
		.depthBiasClamp = 0.0f, // Optional
		.depthBiasSlopeFactor = 0.0f, // Optional
		.lineWidth = m_pGraphicsSettings->wireframeThickness
	};
	m_pGraphicsSettings->wireframe ? rasterizer.polygonMode = VK_POLYGON_MODE_LINE : rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	m_pGraphicsSettings->wireframe ? rasterizer.cullMode = VK_CULL_MODE_NONE : rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;

	mDebugPrint("Creating multisampler...");
	VkPipelineMultisampleStateCreateInfo multisampling{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
		//multisampling.sampleShadingEnable = VK_FALSE;
		.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
		.minSampleShading = 1.0f, // Optional
		.pSampleMask = nullptr, // Optional
		.alphaToCoverageEnable = VK_FALSE, // Optional
		.alphaToOneEnable = VK_FALSE, // Optional
	};
	m_pGraphicsSettings->multisampling == true ? multisampling.sampleShadingEnable = VK_TRUE : multisampling.sampleShadingEnable = VK_FALSE;

	mDebugPrint("Creating depth stencil...");
	VkPipelineDepthStencilStateCreateInfo depthStencil{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
		.depthTestEnable = VK_TRUE,
		.depthWriteEnable = VK_TRUE,
		.depthCompareOp = VK_COMPARE_OP_LESS,
		.depthBoundsTestEnable = VK_FALSE,
		.stencilTestEnable = VK_FALSE,
		.front = {}, // Optional
		.back = {}, // Optional
		.minDepthBounds = 0.0f, // Optional
		.maxDepthBounds = 1.0f // Optional
	};

	mDebugPrint("Creating color blender...");
	VkPipelineColorBlendAttachmentState colorBlendAttachment{
		.blendEnable = VK_TRUE,
		.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
		.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
		.colorBlendOp = VK_BLEND_OP_ADD,
		.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
		.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
		.alphaBlendOp = VK_BLEND_OP_ADD,
		.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
	};
	/*
	colorBlendAttachment.blendEnable = VK_FALSE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional
	*/

	VkPipelineColorBlendStateCreateInfo colorBlending{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
		.logicOpEnable = VK_FALSE,
		.logicOp = VK_LOGIC_OP_COPY, // Optional
		.attachmentCount = 1,
		.pAttachments = &colorBlendAttachment
	};
	colorBlending.blendConstants[0] = 0.0f; // Optional
	colorBlending.blendConstants[1] = 0.0f; // Optional
	colorBlending.blendConstants[2] = 0.0f; // Optional
	colorBlending.blendConstants[3] = 0.0f; // Optional


	std::vector<VkDynamicState> dynamicStates = {
	VK_DYNAMIC_STATE_VIEWPORT,
	VK_DYNAMIC_STATE_SCISSOR
	};

	VkPipelineDynamicStateCreateInfo dynamicState{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
		.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
		.pDynamicStates = dynamicStates.data()
	};

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		.setLayoutCount = 1, // Optional
		.pSetLayouts = &m_descriptorSetLayout,
		.pushConstantRangeCount = 0, // Optional 
		.pPushConstantRanges = nullptr // Optional
	};

	if (vkCreatePipelineLayout(*m_pLogicalDevice, &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create pipeline layout!");
	}



	// Pipeline
	mDebugPrint("Creating pipeline...");
	VkGraphicsPipelineCreateInfo pipelineInfo{
		.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
		.stageCount = 2,
		.pStages = shaderStages,
		.pVertexInputState = &vertexInputInfo,
		.pInputAssemblyState = &inputAssembly,
		//pTessellationState = nullptr, // Optional
		.pViewportState = &viewportState,
		.pRasterizationState = &rasterizer,
		.pMultisampleState = &multisampling,
		.pDepthStencilState = &depthStencil,
		.pColorBlendState = &colorBlending,
		.pDynamicState = &dynamicState,
		// Fixed Functions
		.layout = m_pipelineLayout,
		// Render Pass
		.renderPass = m_renderPass,
		.subpass = 0,
		// Derivatives
		.basePipelineHandle = VK_NULL_HANDLE, // Optional
		.basePipelineIndex = -1 // Optional
	};


	if (vkCreateGraphicsPipelines(*m_pLogicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_graphicsPipeline) != VK_SUCCESS) {
		throw std::runtime_error("failed to create graphics pipeline!");
	}

	for (auto shaderStage : shaderStagesV) {
		vkDestroyShaderModule(*m_pLogicalDevice, shaderStage.module, nullptr);
	}
}

void GraphicsPipeline::cleanup()
{
	vkDestroyImageView(*m_pLogicalDevice, m_colorImageView, nullptr);
	vkDestroyImage(*m_pLogicalDevice, m_colorImage, nullptr);
	vkFreeMemory(*m_pLogicalDevice, m_colorImageMemory, nullptr);

	vkDestroyImageView(*m_pLogicalDevice, m_depthImageView, nullptr);
	vkDestroyImage(*m_pLogicalDevice, m_depthImage, nullptr);
	vkFreeMemory(*m_pLogicalDevice, m_depthImageMemory, nullptr);

	vkDestroyDescriptorSetLayout(*m_pLogicalDevice, m_descriptorSetLayout, nullptr);
	vkDestroyPipeline(*m_pLogicalDevice, m_graphicsPipeline, nullptr);
	vkDestroyPipelineLayout(*m_pLogicalDevice, m_pipelineLayout, nullptr);
	vkDestroyRenderPass(*m_pLogicalDevice, m_renderPass, nullptr);
}



VkShaderModule GraphicsPipeline::createShaderModule(const std::vector<char>& code)
{
	VkShaderModuleCreateInfo createInfo{
		.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		.codeSize = code.size(),
		.pCode = reinterpret_cast<const uint32_t*>(code.data())
	};

	VkShaderModule shaderModule;
	if (vkCreateShaderModule(*m_pLogicalDevice, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
		throw std::runtime_error("failed to create shader module!");
	}

	return shaderModule;
}

void GraphicsPipeline::createColorResources() {

	VkFormat colorFormat = *m_pSwapchain->getSwapchainImageFormat();
	Image::createImage(m_pSwapchain->getSwapchainExtent()->width, m_pSwapchain->getSwapchainExtent()->height, colorFormat, m_msaaSamples, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_colorImage, m_colorImageMemory);
	m_colorImageView = Image::createImageView(m_colorImage, colorFormat, VK_IMAGE_ASPECT_COLOR_BIT);
}

void GraphicsPipeline::createDepthResources() {
	VkFormat depthFormat = DepthBuffer::findDepthFormat(m_pPhysicalDevice);
	Image::createImage(m_pSwapchain->getSwapchainExtent()->width, m_pSwapchain->getSwapchainExtent()->height, depthFormat, m_msaaSamples, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_depthImage, m_depthImageMemory);
	m_depthImageView = Image::createImageView(m_depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
}
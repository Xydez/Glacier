#include "graphics/Pipeline.hpp"
#include "core/Application.hpp"
#include "graphics/Renderer.hpp"

#include <vulkan/vulkan.h>
#include <spdlog/spdlog.h>
#include <glm/glm.hpp>

glacier::Pipeline::Pipeline(const glacier::Application* application, const glacier::Renderer* renderer, const std::unordered_map<ShaderType, Shader*>& shaders, const std::optional<UniformBuffer*>& uniformBuffer, const VertexBuffer* vertexBuffer, const IndexBuffer* indexBuffer)
	: m_Application(application), m_VertexBuffer(vertexBuffer), m_IndexBuffer(indexBuffer), m_Shaders(shaders), m_UniformBuffer(uniformBuffer)
{
	glacier::g_Logger->trace("Creating pipeline...");

	std::vector<VkPipelineShaderStageCreateInfo> shaderStages;

	bool hasVertex = false, hasFragment = false;

	for (const std::pair<ShaderType, Shader*>& pair : shaders)
	{
		VkPipelineShaderStageCreateInfo shaderCreateInfo = {};
		shaderCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;

		switch (pair.first)
		{
		case ShaderType::Vertex:
			hasVertex = true;
			shaderCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
			break;
		case ShaderType::Fragment:
			hasFragment = true;
			shaderCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
			break;
		}

		shaderCreateInfo.module = static_cast<VkShaderModule>(pair.second->m_ShaderModule);
		shaderCreateInfo.pName = "main";

		shaderStages.push_back(shaderCreateInfo);
	}

	if (!hasVertex || !hasFragment)
		throw std::runtime_error("At least one vertex and fragment shader must exist");

	VkPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo = {};
	inputAssemblyCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssemblyCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssemblyCreateInfo.primitiveRestartEnable = VK_FALSE;

	VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo = {};
	vertexInputCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

	// TODO: Vertex buffers

	// EDIT
	//vertexInputCreateInfo.vertexBindingDescriptionCount = 0;
	//vertexInputCreateInfo.pVertexBindingDescriptions = nullptr;
	//vertexInputCreateInfo.vertexAttributeDescriptionCount = 0;
	//vertexInputCreateInfo.pVertexAttributeDescriptions = nullptr;
	// ->

	vertexInputCreateInfo.vertexBindingDescriptionCount = 1;
	VkVertexInputBindingDescription bindingDescription = vertexBuffer->m_Layout.getBindingDescription();
	vertexInputCreateInfo.pVertexBindingDescriptions = &bindingDescription;
	
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions = vertexBuffer->m_Layout.getAttributeDescriptions();
	vertexInputCreateInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
	vertexInputCreateInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

	/*
	 * float:	VK_FORMAT_R32_SFLOAT
	 * vec2:	VK_FORMAT_R32G32_SFLOAT
	 * vec3:	VK_FORMAT_R32G32B32_SFLOAT
	 * vec4:	VK_FORMAT_R32G32B32A32_SFLOAT
	 */
	 // END

	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;

	glm::uvec2 size = application->m_Window->getFramebufferSize();
	viewport.width = static_cast<float>(size.x);
	viewport.height = static_cast<float>(size.y);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor = {};
	scissor.offset = { 0, 0 };
	scissor.extent = { size.x, size.y };

	VkPipelineViewportStateCreateInfo viewportCreateInfo = {};
	viewportCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportCreateInfo.viewportCount = 1;
	viewportCreateInfo.pViewports = &viewport;
	viewportCreateInfo.scissorCount = 1;
	viewportCreateInfo.pScissors = &scissor;

	VkPipelineRasterizationStateCreateInfo rasterizerCreateInfo = {};
	rasterizerCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizerCreateInfo.depthClampEnable = VK_FALSE;
	rasterizerCreateInfo.rasterizerDiscardEnable = VK_FALSE;
	rasterizerCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizerCreateInfo.lineWidth = 1.0f;

	// Disable face culling
	rasterizerCreateInfo.cullMode = VK_CULL_MODE_NONE; // VK_CULL_MODE_BACK_BIT
	rasterizerCreateInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;

	rasterizerCreateInfo.depthBiasEnable = VK_FALSE;
	rasterizerCreateInfo.depthBiasConstantFactor = 0.0f;
	rasterizerCreateInfo.depthBiasClamp = 0.0f;
	rasterizerCreateInfo.depthBiasSlopeFactor = 0.0f;

	VkPipelineMultisampleStateCreateInfo multisampleCreateInfo = {};
	multisampleCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampleCreateInfo.sampleShadingEnable = VK_FALSE;
	multisampleCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampleCreateInfo.minSampleShading = 1.0f;
	multisampleCreateInfo.pSampleMask = nullptr;
	multisampleCreateInfo.alphaToCoverageEnable = VK_FALSE;
	multisampleCreateInfo.alphaToOneEnable = VK_FALSE;

	VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;

	VkPipelineColorBlendStateCreateInfo colorBlendCreateInfo = {};
	colorBlendCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlendCreateInfo.logicOpEnable = VK_FALSE;
	colorBlendCreateInfo.logicOp = VK_LOGIC_OP_COPY;
	colorBlendCreateInfo.attachmentCount = 1;
	colorBlendCreateInfo.pAttachments = &colorBlendAttachment;
	colorBlendCreateInfo.blendConstants[0] = 0.0f;
	colorBlendCreateInfo.blendConstants[1] = 0.0f;
	colorBlendCreateInfo.blendConstants[2] = 0.0f;
	colorBlendCreateInfo.blendConstants[3] = 0.0f;

	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
	pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCreateInfo.setLayoutCount = 1;
	pipelineLayoutCreateInfo.pSetLayouts = reinterpret_cast<const VkDescriptorSetLayout*>(&m_Application->m_UniformBufferLayout);
	pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
	pipelineLayoutCreateInfo.pPushConstantRanges = nullptr;

	if (vkCreatePipelineLayout(static_cast<VkDevice>(application->m_Device), &pipelineLayoutCreateInfo, nullptr, reinterpret_cast<VkPipelineLayout*>(&m_PipelineLayout)) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create pipeline layout");
	}

	/* Create graphics pipeline */
	VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo = {};
	graphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;

	// Shader stages
	graphicsPipelineCreateInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
	graphicsPipelineCreateInfo.pStages = shaderStages.data();

	graphicsPipelineCreateInfo.pVertexInputState = &vertexInputCreateInfo;
	graphicsPipelineCreateInfo.pInputAssemblyState = &inputAssemblyCreateInfo;
	graphicsPipelineCreateInfo.pViewportState = &viewportCreateInfo;
	graphicsPipelineCreateInfo.pRasterizationState = &rasterizerCreateInfo;
	graphicsPipelineCreateInfo.pMultisampleState = &multisampleCreateInfo;
	graphicsPipelineCreateInfo.pDepthStencilState = nullptr;
	graphicsPipelineCreateInfo.pColorBlendState = &colorBlendCreateInfo;

	// TODO: Implement dynamic state (To change viewport size)
	graphicsPipelineCreateInfo.pDynamicState = nullptr;

	graphicsPipelineCreateInfo.layout = static_cast<VkPipelineLayout>(m_PipelineLayout);

	graphicsPipelineCreateInfo.renderPass = static_cast<VkRenderPass>(renderer->m_RenderPass);
	graphicsPipelineCreateInfo.subpass = 0;

	graphicsPipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
	graphicsPipelineCreateInfo.basePipelineIndex = -1;

	if (vkCreateGraphicsPipelines(static_cast<VkDevice>(m_Application->m_Device), VK_NULL_HANDLE, 1, &graphicsPipelineCreateInfo, nullptr, reinterpret_cast<VkPipeline*>(&m_Pipeline)) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create graphics pipeline");
	}
}

glacier::Pipeline::~Pipeline()
{
	vkDestroyPipeline(static_cast<VkDevice>(m_Application->m_Device), static_cast<VkPipeline>(m_Pipeline), nullptr);
	vkDestroyPipelineLayout(static_cast<VkDevice>(m_Application->m_Device), static_cast<VkPipelineLayout>(m_PipelineLayout), nullptr);
}

#include "graphics/Shader.hpp"
#include "core/Application.hpp"
#include "io/File.hpp"

#include <fstream>

#include <spdlog/fmt/fmt.h>
#include <vulkan/vulkan.h>

glacier::Shader::Shader(const Application* application, std::string_view path)
	: m_Application(application)
{
	/* Read shader from file */
	Buffer buffer = File(path).read();

	/* Create shader module */
	VkShaderModuleCreateInfo shaderCreateInfo = { };
	shaderCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	shaderCreateInfo.codeSize = buffer.size();
	shaderCreateInfo.pCode = reinterpret_cast<const uint32_t*>(buffer.data());

	if (vkCreateShaderModule(static_cast<VkDevice>(m_Application->m_Device), &shaderCreateInfo, nullptr, reinterpret_cast<VkShaderModule*>(&m_ShaderModule)) != VK_SUCCESS)
	{
		throw std::runtime_error(fmt::format("Failed to create shader {}", path));
	}
}

glacier::Shader::Shader(const Application* application, const Buffer& buffer)
	: m_Application(application)
{
	/* Create shader module */
	VkShaderModuleCreateInfo shaderCreateInfo = { };
	shaderCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	shaderCreateInfo.codeSize = buffer.size();
	shaderCreateInfo.pCode = reinterpret_cast<const uint32_t*>(buffer.data());

	if (vkCreateShaderModule(static_cast<VkDevice>(m_Application->m_Device), &shaderCreateInfo, nullptr, reinterpret_cast<VkShaderModule*>(&m_ShaderModule)) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create shader");
	}
}

glacier::Shader::~Shader()
{
	vkDestroyShaderModule(static_cast<VkDevice>(m_Application->m_Device), static_cast<VkShaderModule>(m_ShaderModule), nullptr);
}

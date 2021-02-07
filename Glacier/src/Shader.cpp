#include "Shader.hpp"
#include "Application.hpp"

#include <fstream>

#include <spdlog/fmt/fmt.h>
#include <vulkan/vulkan.h>

glacier::Shader::Shader(const Application& application, std::string_view path, const ShaderType& type)
	: m_Application(application), m_Type(type)
{
	std::ifstream stream(path.data(), std::ios::ate | std::ios::binary);

	if (!stream)
	{
		throw std::runtime_error(fmt::format("Failed to load shader {}", path));
	}

	size_t size = stream.tellg();
	stream.seekg(0);

	std::vector<char> buffer(size);
	stream.read(buffer.data(), size);

	stream.close();

	/* Create shader module */
	VkShaderModuleCreateInfo shaderCreateInfo = { };
	shaderCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	shaderCreateInfo.codeSize = buffer.size();
	shaderCreateInfo.pCode = reinterpret_cast<const uint32_t*>(buffer.data());

	if (vkCreateShaderModule(static_cast<VkDevice>(m_Application.m_Device), &shaderCreateInfo, nullptr, reinterpret_cast<VkShaderModule*>(&m_ShaderModule)) != VK_SUCCESS)
	{
		throw std::runtime_error(fmt::format("Failed to create shader {}", path));
	}
}

glacier::Shader::~Shader()
{
	vkDestroyShaderModule(static_cast<VkDevice>(m_Application.m_Device), static_cast<VkShaderModule>(m_ShaderModule), nullptr);
}

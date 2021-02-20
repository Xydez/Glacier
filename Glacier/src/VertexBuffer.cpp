#include "VertexBuffer.hpp"
#include "Application.hpp"

#include <stdexcept>
#include <vector>
#include <vulkan/vulkan.h>

// https://vulkan-tutorial.com/Vertex_buffers/Vertex_input_description

uint32_t findMemoryType(const VkPhysicalDevice& physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags flags)
{
	VkPhysicalDeviceMemoryProperties memoryProperties;
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);

	for (unsigned int i = 0; i < memoryProperties.memoryTypeCount; i++)
	{
		if (typeFilter & (1 << i) && (memoryProperties.memoryTypes[i].propertyFlags & flags) == flags)
		{
			return i;
		}
	}

	throw std::runtime_error("Failed to find a suitable memory type");
}

glacier::VertexBuffer::VertexBuffer(const Application& application, const void* data, uint32_t size, const VertexBufferLayout& layout)
	: m_Layout(layout), m_Application(&application)
{
	/* Create the buffer */
	VkBufferCreateInfo bufferCreateInfo = {};
	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.size = size;
	bufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	bufferCreateInfo.flags = 0;

	if (vkCreateBuffer(static_cast<VkDevice>(m_Application->m_Device), &bufferCreateInfo, nullptr, reinterpret_cast<VkBuffer*>(&m_Handle)) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create vertex buffer");
	}

	/* Allocate memory for the buffer */
	VkMemoryRequirements memoryRequirements;
	vkGetBufferMemoryRequirements(static_cast<VkDevice>(m_Application->m_Device), static_cast<VkBuffer>(m_Handle), &memoryRequirements);

	VkMemoryAllocateInfo allocateInfo = {};
	allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocateInfo.allocationSize = memoryRequirements.size;
	allocateInfo.memoryTypeIndex = findMemoryType(static_cast<VkPhysicalDevice>(m_Application->m_PhysicalDevice), memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	if (vkAllocateMemory(static_cast<VkDevice>(m_Application->m_Device), &allocateInfo, nullptr, reinterpret_cast<VkDeviceMemory*>(&m_Memory)) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to allocate device memory for vertex buffer");
	}

	vkBindBufferMemory(static_cast<VkDevice>(m_Application->m_Device), static_cast<VkBuffer>(m_Handle), static_cast<VkDeviceMemory>(m_Memory), 0);

	/* Copy data to the buffer */
	void* tmp;
	vkMapMemory(static_cast<VkDevice>(m_Application->m_Device), static_cast<VkDeviceMemory>(m_Memory), 0, size, 0, &tmp);
	memcpy(tmp, data, size);
	vkUnmapMemory(static_cast<VkDevice>(m_Application->m_Device), static_cast<VkDeviceMemory>(m_Memory));
}

glacier::VertexBuffer::~VertexBuffer()
{
	/* Destroy the buffer and free its memory */
	vkDestroyBuffer(static_cast<VkDevice>(m_Application->m_Device), static_cast<VkBuffer>(m_Handle), nullptr);
	vkFreeMemory(static_cast<VkDevice>(m_Application->m_Device), static_cast<VkDeviceMemory>(m_Memory), nullptr);
}

void glacier::VertexBufferLayout::push(glacier::VertexBufferElement elementType, uint32_t count)
{
	m_Elements.push_back(std::make_pair(elementType, count));
}

VkVertexInputBindingDescription glacier::VertexBufferLayout::getBindingDescription() const
{
	VkVertexInputBindingDescription vertexInputBindingDescription = {};
	vertexInputBindingDescription.binding = 0;

	unsigned int stride = 0;
	for (const std::pair<glacier::VertexBufferElement, uint32_t>& pair : m_Elements)
	{
		switch (pair.first)
		{
		case glacier::VertexBufferElement::Byte:
		case glacier::VertexBufferElement::UnsignedByte:
			stride += 1 * pair.second;
			break;
		case glacier::VertexBufferElement::Float:
			stride += 4 * pair.second;
			break;
		case glacier::VertexBufferElement::Int:
		case glacier::VertexBufferElement::UnsignedInt:
			stride += 4 * pair.second;
			break;
		}
	}

	vertexInputBindingDescription.stride = stride; // TODO: Vertex buffer layout
	vertexInputBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX; // Instanced rendering: VK_VERTEX_INPUT_RATE_INSTANCE

	return vertexInputBindingDescription;
}

std::vector<VkVertexInputAttributeDescription> glacier::VertexBufferLayout::getAttributeDescriptions() const
{
	std::vector<VkVertexInputAttributeDescription> vertexInputAttributeDescriptions;

	unsigned int i = 0;
	unsigned int offset = 0;
	for (const std::pair<glacier::VertexBufferElement, size_t>& pair : m_Elements)
	{
		VkVertexInputAttributeDescription description = {};
		description.binding = 0; // TODO
		description.location = i;

		if (pair.second > 4)
			throw std::runtime_error("Too many elements!");

		description.offset = offset;

		switch (pair.first)
		{
		case glacier::VertexBufferElement::Byte:
			switch (pair.second)
			{
			case 1:
				description.format = VK_FORMAT_R8_SINT;
				break;
			case 2:
				description.format = VK_FORMAT_R8G8_SINT;
				break;
			case 3:
				description.format = VK_FORMAT_R8G8B8_SINT;
				break;
			case 4:
				description.format = VK_FORMAT_R8G8B8A8_SINT;
				break;
			}
			offset += 1 * pair.second;
			break;
		case glacier::VertexBufferElement::UnsignedByte:
			switch (pair.second)
			{
			case 1:
				description.format = VK_FORMAT_R8_UINT;
				break;
			case 2:
				description.format = VK_FORMAT_R8G8_UINT;
				break;
			case 3:
				description.format = VK_FORMAT_R8G8B8_UINT;
				break;
			case 4:
				description.format = VK_FORMAT_R8G8B8A8_UINT;
				break;
			}
			offset += 1 * pair.second;
			break;
		case glacier::VertexBufferElement::Int:
			switch (pair.second)
			{
			case 1:
				description.format = VK_FORMAT_R32_SINT;
				break;
			case 2:
				description.format = VK_FORMAT_R32G32_SINT;
				break;
			case 3:
				description.format = VK_FORMAT_R32G32B32_SINT;
				break;
			case 4:
				description.format = VK_FORMAT_R32G32B32A32_SINT;
				break;
			}
			offset += 4 * pair.second;
			break;
		case glacier::VertexBufferElement::UnsignedInt:
			description.format = VK_FORMAT_R32_UINT;
			switch (pair.second)
			{
			case 1:
				description.format = VK_FORMAT_R32_UINT;
				break;
			case 2:
				description.format = VK_FORMAT_R32G32_UINT;
				break;
			case 3:
				description.format = VK_FORMAT_R32G32B32_UINT;
				break;
			case 4:
				description.format = VK_FORMAT_R32G32B32A32_UINT;
				break;
			}
			offset += 4 * pair.second;
			break;
		case glacier::VertexBufferElement::Float:
			description.format = VK_FORMAT_R32_SFLOAT;
			switch (pair.second)
			{
			case 1:
				description.format = VK_FORMAT_R32_SFLOAT;
				break;
			case 2:
				description.format = VK_FORMAT_R32G32_SFLOAT;
				break;
			case 3:
				description.format = VK_FORMAT_R32G32B32_SFLOAT;
				break;
			case 4:
				description.format = VK_FORMAT_R32G32B32A32_SFLOAT;
				break;
			}
			offset += 4 * pair.second;
			break;
		}

		vertexInputAttributeDescriptions.push_back(description);

		i += 1;
	}

	return vertexInputAttributeDescriptions;
}

#include "VertexBuffer.hpp"
#include "Application.hpp"
#include "Renderer.hpp"
#include "internal/utility.hpp"

#include <stdexcept>
#include <vector>
#include <vulkan/vulkan.h>

// https://vulkan-tutorial.com/Vertex_buffers/Vertex_input_description

glacier::VertexBuffer::VertexBuffer(const Application* application, const void* data, uint64_t size, const VertexBufferLayout& layout)
	: m_Layout(layout), m_Application(application)
{
	/* Create a staging buffer */
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	createBuffer(static_cast<VkDevice>(m_Application->m_Device), static_cast<VkPhysicalDevice>(m_Application->m_PhysicalDevice), size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer, &stagingBufferMemory);

	/* Copy data to the staging buffer */
	void* tmp;
	vkMapMemory(static_cast<VkDevice>(m_Application->m_Device), stagingBufferMemory, 0, size, 0, &tmp);
	memcpy(tmp, data, size);
	vkUnmapMemory(static_cast<VkDevice>(m_Application->m_Device), stagingBufferMemory);

	/* Copy data from the staging buffer to the vertex buffer on the GPU */
	createBuffer(static_cast<VkDevice>(m_Application->m_Device), static_cast<VkPhysicalDevice>(m_Application->m_PhysicalDevice), size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, reinterpret_cast<VkBuffer*>(&m_Handle), reinterpret_cast<VkDeviceMemory*>(&m_Memory));

	copyBuffers(static_cast<VkDevice>(m_Application->m_Device), static_cast<VkCommandPool>(m_Application->m_Renderer->m_CommandPool), static_cast<VkQueue>(m_Application->m_Renderer->m_GraphicsQueue), &stagingBuffer, reinterpret_cast<VkBuffer*>(&m_Handle), &size, 1);

	vkDestroyBuffer(static_cast<VkDevice>(m_Application->m_Device), stagingBuffer, nullptr);
	vkFreeMemory(static_cast<VkDevice>(m_Application->m_Device), stagingBufferMemory, nullptr);
}

glacier::VertexBuffer::~VertexBuffer()
{
	/* Destroy the buffer and free its memory */

	if (m_Handle)
		vkDestroyBuffer(static_cast<VkDevice>(m_Application->m_Device), static_cast<VkBuffer>(m_Handle), nullptr);

	if (m_Memory)
		vkFreeMemory(static_cast<VkDevice>(m_Application->m_Device), static_cast<VkDeviceMemory>(m_Memory), nullptr);
}

void glacier::VertexBufferLayout::push(glacier::BufferElement elementType, uint32_t count)
{
	m_Elements.push_back(std::make_pair(elementType, count));
}

VkVertexInputBindingDescription glacier::VertexBufferLayout::getBindingDescription() const
{
	VkVertexInputBindingDescription vertexInputBindingDescription = {};
	vertexInputBindingDescription.binding = 0;

	unsigned int stride = 0;
	for (const std::pair<glacier::BufferElement, uint32_t>& pair : m_Elements)
	{
		switch (pair.first)
		{
		case glacier::BufferElement::Byte:
		case glacier::BufferElement::UnsignedByte:
			stride += 1 * pair.second;
			break;
		case glacier::BufferElement::Float:
			stride += 4 * pair.second;
			break;
		case glacier::BufferElement::Int:
		case glacier::BufferElement::UnsignedInt:
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
	for (const std::pair<glacier::BufferElement, uint32_t>& pair : m_Elements)
	{
		VkVertexInputAttributeDescription description = {};
		description.binding = 0; // TODO
		description.location = i;

		if (pair.second > 4)
			throw std::runtime_error("Too many elements!");

		description.offset = offset;

		switch (pair.first)
		{
		case glacier::BufferElement::Byte:
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
		case glacier::BufferElement::UnsignedByte:
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
		case glacier::BufferElement::Int:
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
		case glacier::BufferElement::UnsignedInt:
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
		case glacier::BufferElement::Float:
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

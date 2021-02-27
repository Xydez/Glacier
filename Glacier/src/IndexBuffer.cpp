#include "IndexBuffer.hpp"
#include "Application.hpp"
#include "Renderer.hpp"
#include "internal/utility.hpp"

#include <stdexcept>
#include <vulkan/vulkan.h>

glacier::IndexBuffer::IndexBuffer(const Application* application, const uint32_t* data, uint64_t size)
	: m_Application(application)
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

	/* Copy data from the staging buffer to the index buffer on the GPU */
	createBuffer(static_cast<VkDevice>(m_Application->m_Device), static_cast<VkPhysicalDevice>(m_Application->m_PhysicalDevice), size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, reinterpret_cast<VkBuffer*>(&m_Handle), reinterpret_cast<VkDeviceMemory*>(&m_Memory));

	copyBuffers(static_cast<VkDevice>(m_Application->m_Device), static_cast<VkCommandPool>(m_Application->m_Renderer->m_CommandPool), static_cast<VkQueue>(m_Application->m_Renderer->m_GraphicsQueue), &stagingBuffer, reinterpret_cast<VkBuffer*>(&m_Handle), &size, 1);

	vkDestroyBuffer(static_cast<VkDevice>(m_Application->m_Device), stagingBuffer, nullptr);
	vkFreeMemory(static_cast<VkDevice>(m_Application->m_Device), stagingBufferMemory, nullptr);
}

glacier::IndexBuffer::~IndexBuffer()
{
	if (m_Handle)
		vkDestroyBuffer(static_cast<VkDevice>(m_Application->m_Device), static_cast<VkBuffer>(m_Handle), nullptr);
	
	if (m_Memory)
		vkFreeMemory(static_cast<VkDevice>(m_Application->m_Device), static_cast<VkDeviceMemory>(m_Memory), nullptr);
}

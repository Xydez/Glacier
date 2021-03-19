#include "graphics/IndexBuffer.hpp"
#include "core/Application.hpp"
#include "graphics/Renderer.hpp"
#include "internal/utility.hpp"

#include <stdexcept>
#include <vulkan/vulkan.h>

glacier::IndexBuffer::IndexBuffer(const Application* application, const uint32_t* data, size_t count)
	: m_Application(application), m_Count(count)
{
	size_t size = m_Count * sizeof(uint32_t);

	m_Data = new char[m_Count * sizeof(uint32_t)];
	memcpy(m_Data, data, size);

	create();

	m_Application->m_Renderer->m_LifecycleObjects.push_back(this);
}

glacier::IndexBuffer::~IndexBuffer()
{
	for (std::vector<LifecycleObject*>::iterator iter = m_Application->m_Renderer->m_LifecycleObjects.begin(); iter != m_Application->m_Renderer->m_LifecycleObjects.end(); iter++)
		if (*iter == this)
		{
			m_Application->m_Renderer->m_LifecycleObjects.erase(iter);
			break;
		}

	delete[] m_Data;

	destroy();
}

void glacier::IndexBuffer::create()
{
	size_t size = m_Count * sizeof(uint32_t);

	/* Create a staging buffer */
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	createBuffer(static_cast<VkDevice>(m_Application->m_Device), static_cast<VkPhysicalDevice>(m_Application->m_PhysicalDevice), size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer, &stagingBufferMemory);

	/* Copy data to the staging buffer */
	void* tmp;
	vkMapMemory(static_cast<VkDevice>(m_Application->m_Device), stagingBufferMemory, 0, size, 0, &tmp);
	memcpy(tmp, m_Data, size);
	vkUnmapMemory(static_cast<VkDevice>(m_Application->m_Device), stagingBufferMemory);

	/* Copy data from the staging buffer to the index buffer on the GPU */
	createBuffer(static_cast<VkDevice>(m_Application->m_Device), static_cast<VkPhysicalDevice>(m_Application->m_PhysicalDevice), size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, reinterpret_cast<VkBuffer*>(&m_Handle), reinterpret_cast<VkDeviceMemory*>(&m_Memory));

	copyBuffers(static_cast<VkDevice>(m_Application->m_Device), static_cast<VkCommandPool>(m_Application->m_Renderer->m_CommandPool), static_cast<VkQueue>(m_Application->m_Renderer->m_GraphicsQueue), &stagingBuffer, reinterpret_cast<VkBuffer*>(&m_Handle), &size, 1);

	vkDestroyBuffer(static_cast<VkDevice>(m_Application->m_Device), stagingBuffer, nullptr);
	vkFreeMemory(static_cast<VkDevice>(m_Application->m_Device), stagingBufferMemory, nullptr);
}

void glacier::IndexBuffer::destroy()
{
	if (m_Handle)
		vkDestroyBuffer(static_cast<VkDevice>(m_Application->m_Device), static_cast<VkBuffer>(m_Handle), nullptr);

	if (m_Memory)
		vkFreeMemory(static_cast<VkDevice>(m_Application->m_Device), static_cast<VkDeviceMemory>(m_Memory), nullptr);
}

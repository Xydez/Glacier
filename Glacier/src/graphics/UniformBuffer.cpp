#include "graphics/UniformBuffer.hpp"
#include "core/Application.hpp"
#include "graphics/Renderer.hpp"

#include "internal/utility.hpp"

#include <vulkan/vulkan.h>
#include <stdexcept>
#include <spdlog/fmt/fmt.h>

// TODO: Create and destroy UniformBuffer and shit
// https://vulkan-tutorial.com/Uniform_buffers/Descriptor_layout_and_buffer#page_Uniform-buffer


glacier::UniformBuffer::UniformBuffer(const Application* application, size_t size)
    : m_Application(application), m_Size(size)
{
    create();

    //m_Application->m_Renderer->m_LifecycleObjects.push_back(this);
}

glacier::UniformBuffer::~UniformBuffer()
{
    //vkFreeDescriptorSets(static_cast<VkDevice>(m_Application->m_Device), static_cast<VkDescriptorPool>(m_Application->m_Renderer->m_DescriptorPool), m_DescriptorSets.size(), reinterpret_cast<VkDescriptorSet*>(m_DescriptorSets.data()));
    destroy();
}

void glacier::UniformBuffer::create()
{
    size_t imageCount = m_Application->m_Renderer->m_Images.size();

    m_HandleVector.resize(imageCount);
    m_MemoryVector.resize(imageCount);

    for (unsigned int i = 0; i < imageCount; i++)
    {
        createBuffer(static_cast<VkDevice>(m_Application->m_Device), static_cast<VkPhysicalDevice>(m_Application->m_PhysicalDevice), m_Size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, reinterpret_cast<VkBuffer*>(&m_HandleVector[i]), reinterpret_cast<VkDeviceMemory*>(&m_MemoryVector[i]));
    }

    /* Create desciptor sets */
    std::vector<VkDescriptorSetLayout> layouts(imageCount, static_cast<VkDescriptorSetLayout>(m_Application->m_UniformBufferLayout));

    VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = {};
    descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptorSetAllocateInfo.descriptorPool = static_cast<VkDescriptorPool>(m_Application->m_Renderer->m_DescriptorPool);
    descriptorSetAllocateInfo.descriptorSetCount = static_cast<uint32_t>(imageCount);
    descriptorSetAllocateInfo.pSetLayouts = layouts.data();

    m_DescriptorSets.resize(imageCount);
    VkResult result = vkAllocateDescriptorSets(static_cast<VkDevice>(m_Application->m_Device), &descriptorSetAllocateInfo, reinterpret_cast<VkDescriptorSet*>(m_DescriptorSets.data()));
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error(fmt::format("Failed to allocate descriptor sets (Returned {})", result));
    }

    for (unsigned int i = 0; i < imageCount; i++)
    {
        VkDescriptorBufferInfo bufferInfo = {};
        bufferInfo.buffer = static_cast<VkBuffer>(m_HandleVector[i]);
        bufferInfo.offset = 0;
        bufferInfo.range = m_Size;

        VkWriteDescriptorSet writeDescriptorSet = {};
        writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDescriptorSet.dstSet = static_cast<VkDescriptorSet>(m_DescriptorSets[i]);
        writeDescriptorSet.dstBinding = 0;
        writeDescriptorSet.dstArrayElement = 0;
        writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        writeDescriptorSet.descriptorCount = 1;
        writeDescriptorSet.pBufferInfo = &bufferInfo;
        writeDescriptorSet.pImageInfo = nullptr;
        writeDescriptorSet.pTexelBufferView = nullptr;

        vkUpdateDescriptorSets(static_cast<VkDevice>(m_Application->m_Device), 1, &writeDescriptorSet, 0, nullptr);
    }
}

void glacier::UniformBuffer::destroy()
{
    for (unsigned int i = 0; i < m_HandleVector.size(); i++)
        vkDestroyBuffer(static_cast<VkDevice>(m_Application->m_Device), static_cast<VkBuffer>(m_HandleVector[i]), nullptr);

    for (unsigned int i = 0; i < m_MemoryVector.size(); i++)
        vkFreeMemory(static_cast<VkDevice>(m_Application->m_Device), static_cast<VkDeviceMemory>(m_MemoryVector[i]), nullptr);
}

void glacier::UniformBuffer::set(const void* data)
{
    void* tmp;
    
    for (unsigned int i = 0; i < m_MemoryVector.size(); i++)
    {
        vkMapMemory(static_cast<VkDevice>(m_Application->m_Device), static_cast<VkDeviceMemory>(m_MemoryVector[i]), 0, m_Size, 0, &tmp);
        memcpy(tmp, data, m_Size);
        vkUnmapMemory(static_cast<VkDevice>(m_Application->m_Device), static_cast<VkDeviceMemory>(m_MemoryVector[i]));
    }
}

void glacier::UniformBuffer::update(const void* data)
{
    void* tmp;

    vkMapMemory(static_cast<VkDevice>(m_Application->m_Device), static_cast<VkDeviceMemory>(m_MemoryVector[m_Application->m_CurrentFrame]), 0, m_Size, 0, &tmp);
    memcpy(tmp, data, m_Size);
    vkUnmapMemory(static_cast<VkDevice>(m_Application->m_Device), static_cast<VkDeviceMemory>(m_MemoryVector[m_Application->m_CurrentFrame]));
}

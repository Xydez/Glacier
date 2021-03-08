#include "internal/utility.hpp"

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

void createBuffer(const VkDevice& device, const VkPhysicalDevice& physicalDevice, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags flags, VkBuffer* buffer, VkDeviceMemory* memory)
{
	/* Create the buffer */
	VkBufferCreateInfo bufferCreateInfo = {};
	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.size = size;
	bufferCreateInfo.usage = usage;
	bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(device, &bufferCreateInfo, nullptr, buffer) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create buffer");
	}

	/* Allocate memory for the buffer */
	VkMemoryRequirements memoryRequirements;
	vkGetBufferMemoryRequirements(device, *buffer, &memoryRequirements);

	VkMemoryAllocateInfo memoryAllocateInfo = {};
	memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocateInfo.allocationSize = memoryRequirements.size;
	memoryAllocateInfo.memoryTypeIndex = findMemoryType(physicalDevice, memoryRequirements.memoryTypeBits, flags);

	if (vkAllocateMemory(device, &memoryAllocateInfo, nullptr, memory) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to allocate memory for buffer");
	}

	/* Bind device memory to the buffer */
	vkBindBufferMemory(device, *buffer, *memory, 0);
}

void copyBuffers(const VkDevice& device, const VkCommandPool& commandPool, const VkQueue& graphicsQueue, const VkBuffer* srcBuffers, const VkBuffer* dstBuffers, const VkDeviceSize* bufferSizes, unsigned int bufferCount)
{
	/* Create a temporary command buffer to copy the data */
	VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
	commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	commandBufferAllocateInfo.commandPool = commandPool;
	commandBufferAllocateInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(device, &commandBufferAllocateInfo, &commandBuffer);

	VkCommandBufferBeginInfo commandBufferBeginInfo = {};
	commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo);

	for (unsigned int i = 0; i < bufferCount; i++)
	{
		VkBufferCopy copyRegion = {};
		copyRegion.srcOffset = 0;
		copyRegion.dstOffset = 0;
		copyRegion.size = bufferSizes[i];
		vkCmdCopyBuffer(commandBuffer, srcBuffers[i], dstBuffers[i], 1, &copyRegion);
	}

	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(graphicsQueue);

	vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}

QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface)
{
	QueueFamilyIndices queueFamilyIndices;

	unsigned int queueFamilyCount;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilyProperties.data());

	unsigned int i = 0;
	for (const VkQueueFamilyProperties& properties : queueFamilyProperties)
	{
		if (properties.queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			queueFamilyIndices.graphicsFamily = i;
		}

		VkBool32 presentSupport = VK_FALSE;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

		if (presentSupport)
		{
			queueFamilyIndices.presentationFamily = i;
		}

		// If all the queue families are found, break the loop
		if (queueFamilyIndices.isComplete())
			break;

		i++;
	}

	return queueFamilyIndices;
}

SwapchainSupportDetails querySwapchainSupport(VkPhysicalDevice device, VkSurfaceKHR surface)
{
	SwapchainSupportDetails details = {};

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &(details.capabilities));

	unsigned int formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

	if (formatCount > 0)
	{
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
	}

	unsigned int presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

	if (presentModeCount > 0)
	{
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
	}

	return details;
}

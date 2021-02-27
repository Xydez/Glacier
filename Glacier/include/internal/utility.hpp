#pragma once

#include <vulkan/vulkan.h>

#include <stdexcept>
#include <vector>
#include <optional>

struct QueueFamilyIndices
{
	std::optional<unsigned int> graphicsFamily;
	std::optional<unsigned int> presentationFamily;

	bool isComplete()
	{
		if (graphicsFamily.has_value() && presentationFamily.has_value())
			return true;

		return false;
	}
};

struct SwapchainSupportDetails
{
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;

	bool isAdequate()
	{
		return (formats.empty() || presentModes.empty()) == false;
	}
};

uint32_t findMemoryType(const VkPhysicalDevice& physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags flags);

void createBuffer(const VkDevice& device, const VkPhysicalDevice& physicalDevice, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags flags, VkBuffer* buffer, VkDeviceMemory* memory);

void copyBuffers(const VkDevice& device, const VkCommandPool& commandPool, const VkQueue& graphicsQueue, VkBuffer* srcBuffers, VkBuffer* dstBuffers, VkDeviceSize* bufferSizes, unsigned int bufferCount);

QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);

SwapchainSupportDetails querySwapchainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);

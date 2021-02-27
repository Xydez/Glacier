#include "Renderer.hpp"
#include "Application.hpp"
#include "Pipeline.hpp"
#include "internal/utility.hpp"

#include <spdlog/spdlog.h>
#include <optional>
#include <vulkan/vulkan.h>

struct ShaderInfo
{
	VkShaderStageFlagBits stage;
	VkShaderModule module;
};

struct BufferInfo
{
	VkVertexInputBindingDescription inputBinding;
	std::vector<VkVertexInputAttributeDescription> inputAttributes;
};

VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats)
{
	for (const VkSurfaceFormatKHR& surfaceFormat : formats)
	{
		if (surfaceFormat.format == VK_FORMAT_B8G8R8A8_SRGB && surfaceFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			return surfaceFormat;
		}
	}

	return formats[0];
}

VkPresentModeKHR choosePresentMode(const std::vector<VkPresentModeKHR>& modes, bool vsync)
{
	for (const VkPresentModeKHR& mode : modes)
	{
		// If vsync is disabled, prefer immediate.
		if (vsync == false && mode == VK_PRESENT_MODE_IMMEDIATE_KHR)
			return mode;

		// If vsync is enabled, prefer triple buffering over double buffering.
		if (vsync == true && mode == VK_PRESENT_MODE_MAILBOX_KHR)
			return mode;
	}

	// If the preferred modes aren't available, default to double buffering.
	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, const glacier::Window& window)
{
	if (capabilities.currentExtent.width != UINT32_MAX)
	{
		// If screen coordinates match pixels, return the current extent in screen coordinates.
		return capabilities.currentExtent;
	}
	else
	{
		// If screen coordinates don't match pixels, get the current extent in pixels from GLFW
		glm::uvec2 size = window.getFramebufferSize();

		VkExtent2D actualExtent = { static_cast<uint32_t>(size.x), static_cast<uint32_t>(size.y) };

		return actualExtent;
	}
}

/* Recreate swapchain */
// Create swapchain
void createSwapchain(const VkPhysicalDevice& physicalDevice, const VkDevice& device, const VkSurfaceKHR& surface, const glacier::ApplicationInfo& applicationInfo, const glacier::Window& window, const SwapchainSupportDetails& details, const VkSurfaceFormatKHR& surfaceFormat, const VkPresentModeKHR& presentMode, const VkExtent2D& extent, const VkSwapchainKHR* oldSwapchain, VkSwapchainKHR* pSwapchain)
{
	/* Create a swap chain */
	glacier::g_Logger->trace("Creating swap chain");

	unsigned int imageCount = details.capabilities.minImageCount + 1;
	if (details.capabilities.minImageCount > 0 && imageCount > details.capabilities.maxImageCount)
	{
		imageCount = details.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR swapchainCreateInfo = {};
	swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainCreateInfo.surface = surface;
	swapchainCreateInfo.minImageCount = imageCount;
	swapchainCreateInfo.imageFormat = surfaceFormat.format;
	swapchainCreateInfo.imageColorSpace = surfaceFormat.colorSpace;
	swapchainCreateInfo.imageExtent = extent;
	swapchainCreateInfo.imageArrayLayers = 1;

	// Images will be written to directly
	swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice, surface);

	uint32_t indices[] = { queueFamilyIndices.graphicsFamily.value(), queueFamilyIndices.presentationFamily.value() };

	if (queueFamilyIndices.graphicsFamily != queueFamilyIndices.presentationFamily)
	{
		// If the graphics and presentation families are different, we need to specify that both share ownership of the image.

		swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		swapchainCreateInfo.queueFamilyIndexCount = 2;
		swapchainCreateInfo.pQueueFamilyIndices = indices;
	}
	else
	{
		// If the graphcis and presentation families are the same, we can safely specify that they exclusively own the image.
		swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		swapchainCreateInfo.queueFamilyIndexCount = 0;
		swapchainCreateInfo.pQueueFamilyIndices = nullptr;
	}

	// Don't transform the image
	swapchainCreateInfo.preTransform = details.capabilities.currentTransform;

	// Ignore the alpha channel
	swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

	swapchainCreateInfo.presentMode = presentMode;

	// Enable clipping of unused pixels for better performance
	swapchainCreateInfo.clipped = VK_TRUE;

	// TODO: If the window is resized, we need to create an entirely new swapchain
	swapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

	if (vkCreateSwapchainKHR(device, &swapchainCreateInfo, nullptr, pSwapchain) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create swapchain");
	}
}

// Create image views
void createImageViews(const VkDevice& device, const VkSurfaceFormatKHR& surfaceFormat, const VkExtent2D& extent, const VkSwapchainKHR& swapchain, std::vector<VkImage>& swapchainImages, std::vector<VkImageView>& imageViews)
{
	glacier::g_Logger->trace("Creating image views...");

	VkFormat swapchainImageFormat = surfaceFormat.format;
	VkExtent2D swapchainExtent = extent;

	uint32_t imageCount;
	vkGetSwapchainImagesKHR(device, swapchain, &imageCount, nullptr);

	swapchainImages.clear();
	swapchainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(device, swapchain, &imageCount, swapchainImages.data());

	imageViews.clear();
	imageViews.resize(swapchainImages.size());
	for (size_t i = 0; i < swapchainImages.size(); i++)
	{
		VkImageViewCreateInfo imageViewCreateInfo = {};
		imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewCreateInfo.image = swapchainImages[i];
		imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		imageViewCreateInfo.format = swapchainImageFormat;
		imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
		imageViewCreateInfo.subresourceRange.levelCount = 1;
		imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
		imageViewCreateInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView(device, &imageViewCreateInfo, nullptr, &(*(imageViews.begin() + i))) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create image view");
		}
	}
}

// Create render pass
void createRenderPass(const VkDevice& device, const VkSurfaceFormatKHR& surfaceFormat, VkRenderPass* renderPass)
{
	glacier::g_Logger->trace("Creating render pass...");

	// Create color attachment (To clear the screen)
	VkAttachmentDescription colorAttachment = {};
	colorAttachment.format = surfaceFormat.format;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

	// We won't do anything with the stencil buffer, so we ignore these
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	// Create attachment reference
	VkAttachmentReference colorAttachmentReference = {};
	colorAttachmentReference.attachment = 0;
	colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	// Create subpass
	VkSubpassDescription subpassDescription = {};
	subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassDescription.colorAttachmentCount = 1;
	subpassDescription.pColorAttachments = &colorAttachmentReference;

	// Create render pass
	VkRenderPassCreateInfo renderPassCreateInfo = {};
	renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassCreateInfo.attachmentCount = 1;
	renderPassCreateInfo.pAttachments = &colorAttachment;
	renderPassCreateInfo.subpassCount = 1;
	renderPassCreateInfo.pSubpasses = &subpassDescription;

	if (vkCreateRenderPass(device, &renderPassCreateInfo, nullptr, renderPass) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create render pass");
	}
}
// Create framebuffers
void createFramebuffers(const VkDevice& device, const std::vector<VkImageView>& imageViews, const VkRenderPass& renderPass, const VkExtent2D& swapchainExtent, std::vector<VkFramebuffer>& framebuffers)
{
	glacier::g_Logger->trace("Creating framebuffers...");

	framebuffers.clear();
	framebuffers.resize(imageViews.size());

	// Create a framebuffer for every image view
	for (size_t i = 0; i < imageViews.size(); i++)
	{
		std::vector<VkImageView> attachments = {
			imageViews[i]
		};

		VkFramebufferCreateInfo framebufferCreateInfo = {};
		framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferCreateInfo.renderPass = renderPass;
		framebufferCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		framebufferCreateInfo.pAttachments = attachments.data();
		framebufferCreateInfo.width = swapchainExtent.width;
		framebufferCreateInfo.height = swapchainExtent.height;
		framebufferCreateInfo.layers = 1;

		if (vkCreateFramebuffer(device, &framebufferCreateInfo, nullptr, &framebuffers[i]) != VK_SUCCESS)
		{
			throw std::runtime_error(fmt::format("Failed to create framebuffer {}", i));
		}
	}
}
// Create command pool
void createCommandPool(const VkDevice& device, const QueueFamilyIndices& queueFamilyIndices, VkCommandPool* commandPool)
{
	glacier::g_Logger->trace("Creating command pool");

	VkCommandPoolCreateInfo commandPoolCreateInfo = {};
	commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolCreateInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
	commandPoolCreateInfo.flags = 0;

	if (vkCreateCommandPool(device, &commandPoolCreateInfo, nullptr, commandPool) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create command pool");
	}
}
// Create command buffers
void createCommandBuffers(const VkDevice& device, const std::vector<VkFramebuffer>& framebuffers, const VkCommandPool& commandPool, std::vector<VkCommandBuffer>& commandBuffers)
{
	glacier::g_Logger->trace("Creating command buffers...");

	commandBuffers.clear();
	commandBuffers.resize(framebuffers.size());

	VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
	commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferAllocateInfo.commandPool = commandPool;
	commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	commandBufferAllocateInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

	if (vkAllocateCommandBuffers(device, &commandBufferAllocateInfo, commandBuffers.data()) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to allocate command buffers");
	}
}
// Destroy swapchain
void destroySwapchain(const VkDevice& device, std::vector<VkFramebuffer>& framebuffers, VkRenderPass* renderPass, std::vector<VkImageView>& imageViews, VkSwapchainKHR* swapchain)
{
	for (size_t i = 0; i < framebuffers.size(); i++)
	{
		vkDestroyFramebuffer(device, framebuffers[i], nullptr);
	}

	// <free command buffers>

	// <destroy pipeline>

	// <destroy pipeline layout>

	vkDestroyRenderPass(device, *renderPass, nullptr);
	*renderPass = nullptr;

	for (size_t i = 0; i < imageViews.size(); i++)
	{
		vkDestroyImageView(device, imageViews[i], nullptr);
	}

	imageViews.clear();

	vkDestroySwapchainKHR(device, *swapchain, nullptr);
	*swapchain = nullptr;
}
/* ------------------ */

void glacier::Renderer::bindPipeline(const Pipeline& pipeline, uint32_t vertices)
{
	/* Unbind old pipeline */
	if (!m_CommandBuffers.empty())
		unbindPipeline();

	/* Create command buffers */
	glm::uvec2 size = m_Application->m_Window->getFramebufferSize();
	VkExtent2D extent = { size.x, size.y };

	createCommandBuffers(static_cast<VkDevice>(m_Application->m_Device), reinterpret_cast<const std::vector<VkFramebuffer>&>(m_Framebuffers), static_cast<VkCommandPool>(m_CommandPool), reinterpret_cast<std::vector<VkCommandBuffer>&>(m_CommandBuffers));

	for (size_t i = 0; i < m_CommandBuffers.size(); i++)
	{
		VkCommandBufferBeginInfo commandBufferBeginInfo = {};
		commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		commandBufferBeginInfo.flags = 0;
		commandBufferBeginInfo.pInheritanceInfo = nullptr;

		if (vkBeginCommandBuffer(static_cast<VkCommandBuffer>(m_CommandBuffers[i]), &commandBufferBeginInfo) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to begin command buffer");
		}

		// Begin render pass
		VkRenderPassBeginInfo renderPassBeginInfo = {};
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.renderPass = static_cast<VkRenderPass>(m_RenderPass);
		renderPassBeginInfo.framebuffer = static_cast<VkFramebuffer>(m_Framebuffers[i]);

		renderPassBeginInfo.renderArea.offset = { 0, 0 };
		renderPassBeginInfo.renderArea.extent = extent;

		VkClearValue clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
		renderPassBeginInfo.clearValueCount = 1;
		renderPassBeginInfo.pClearValues = &clearColor;

		vkCmdBeginRenderPass(static_cast<VkCommandBuffer>(m_CommandBuffers[i]), &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		vkCmdBindPipeline(static_cast<VkCommandBuffer>(m_CommandBuffers[i]), VK_PIPELINE_BIND_POINT_GRAPHICS, static_cast<VkPipeline>(pipeline.m_Pipeline));

		// $ BEGIN $
		VkBuffer vertexBuffers[] = { static_cast<VkBuffer>(pipeline.m_VertexBuffer->m_Handle) };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(static_cast<VkCommandBuffer>(m_CommandBuffers[i]), 0, 1, vertexBuffers, offsets);
		// $ END $

		vkCmdDraw(static_cast<VkCommandBuffer>(m_CommandBuffers[i]), vertices, 1, 0, 0);

		vkCmdEndRenderPass(static_cast<VkCommandBuffer>(m_CommandBuffers[i]));

		if (vkEndCommandBuffer(static_cast<VkCommandBuffer>(m_CommandBuffers[i])) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to end command buffer");
		}
	}

	//TODO Do shit with the command buffers

	// Command buffers are bound, use them in Application.cpp and maybe check if they're bound. Don't clear after render. Unbind function
}

void glacier::Renderer::unbindPipeline()
{
	vkDeviceWaitIdle(static_cast<VkDevice>(m_Application->m_Device));

	vkFreeCommandBuffers(static_cast<VkDevice>(m_Application->m_Device), static_cast<VkCommandPool>(m_CommandPool), static_cast<uint32_t>(m_CommandBuffers.size()), reinterpret_cast<VkCommandBuffer*>(m_CommandBuffers.data()));
	m_CommandBuffers.clear();
}

glacier::Renderer::Renderer(Application* application)
	: m_Application(application), m_Swapchain(nullptr)
{
	vkDeviceWaitIdle(static_cast<VkDevice>(m_Application->m_Device));

	glacier::g_Logger->trace("Creating swapchain...");

	/* Get queue family indices */
	QueueFamilyIndices queueFamilyIndices = findQueueFamilies(static_cast<VkPhysicalDevice>(m_Application->m_PhysicalDevice), static_cast<VkSurfaceKHR>(m_Application->m_Surface));

	/* Create swapchain */
	SwapchainSupportDetails details = querySwapchainSupport(static_cast<VkPhysicalDevice>(m_Application->m_PhysicalDevice), static_cast<VkSurfaceKHR>(m_Application->m_Surface));

	VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(details.formats);
	VkPresentModeKHR presentMode = choosePresentMode(details.presentModes, m_Application->m_Info.vsync);
	VkExtent2D extent = chooseSwapExtent(details.capabilities, *m_Application->m_Window);

	createSwapchain(static_cast<VkPhysicalDevice>(m_Application->m_PhysicalDevice), static_cast<VkDevice>(m_Application->m_Device), static_cast<VkSurfaceKHR>(m_Application->m_Surface), m_Application->m_Info, *m_Application->m_Window, details, surfaceFormat, presentMode, extent, reinterpret_cast<VkSwapchainKHR*>(&m_Swapchain), reinterpret_cast<VkSwapchainKHR*>(&m_Swapchain));

	/* Create image views */
	std::vector<VkImage>* swapchainImages = reinterpret_cast<std::vector<VkImage>*>(&m_Images);
	std::vector<VkImageView>* imageViews = reinterpret_cast<std::vector<VkImageView>*>(&m_ImageViews);
	createImageViews(static_cast<VkDevice>(m_Application->m_Device), surfaceFormat, extent, static_cast<VkSwapchainKHR>(m_Swapchain), *swapchainImages, *imageViews);

	/* Create render pass */
	createRenderPass(static_cast<VkDevice>(m_Application->m_Device), surfaceFormat, reinterpret_cast<VkRenderPass*>(&m_RenderPass));

	/* Create framebuffers */
	std::vector<VkFramebuffer>* framebuffers = reinterpret_cast<std::vector<VkFramebuffer>*>(&m_Framebuffers);
	createFramebuffers(static_cast<VkDevice>(m_Application->m_Device), *imageViews, static_cast<VkRenderPass>(m_RenderPass), extent, *framebuffers);

	createCommandPool(static_cast<VkDevice>(m_Application->m_Device), queueFamilyIndices, reinterpret_cast<VkCommandPool*>(&m_CommandPool));

	/* Get queues */
	vkGetDeviceQueue(static_cast<VkDevice>(m_Application->m_Device), queueFamilyIndices.graphicsFamily.value(), 0, reinterpret_cast<VkQueue*>(&m_GraphicsQueue));
	vkGetDeviceQueue(static_cast<VkDevice>(m_Application->m_Device), queueFamilyIndices.presentationFamily.value(), 0, reinterpret_cast<VkQueue*>(&m_PresentationQueue));
}

glacier::Renderer::~Renderer()
{
	vkDeviceWaitIdle(static_cast<VkDevice>(m_Application->m_Device));

	std::vector<VkFramebuffer>* framebuffers = reinterpret_cast<std::vector<VkFramebuffer>*>(&m_Framebuffers);
	std::vector<VkCommandBuffer>* commandBuffers = reinterpret_cast<std::vector<VkCommandBuffer>*>(&m_CommandBuffers);
	std::vector<VkImageView>* imageViews = reinterpret_cast<std::vector<VkImageView>*>(&m_ImageViews);

	unbindPipeline();

	destroySwapchain(static_cast<VkDevice>(m_Application->m_Device), *framebuffers, reinterpret_cast<VkRenderPass*>(&m_RenderPass), *imageViews, reinterpret_cast<VkSwapchainKHR*>(&m_Swapchain));

	vkDestroyCommandPool(static_cast<VkDevice>(m_Application->m_Device), static_cast<VkCommandPool>(m_CommandPool), nullptr);
}

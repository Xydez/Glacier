#pragma warning(push)
#pragma warning(disable: 26812)

#include "Application.hpp"

#include <vector>
#include <iostream>
#include <set>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

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

struct SwapChainSupportDetails
{
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;

	bool isAdequate()
	{
		return (formats.empty() || presentModes.empty()) == false;
	}
};

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT type, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
	spdlog::level::level_enum level;
	switch (severity)
	{
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
		level = spdlog::level::trace;
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
		level = spdlog::level::info;
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
		level = spdlog::level::warn;
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
		level = spdlog::level::err;
		break;
	default:
		return VK_FALSE;
	}

	if (level == spdlog::level::info)
		return VK_FALSE;

	glacier::g_Logger->log(level, "[Vulkan] {}", pCallbackData->pMessage);
	
	return VK_FALSE;
}

VkResult vkCreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
{
	PFN_vkCreateDebugUtilsMessengerEXT function = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));

	if (function == nullptr)
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	else
		return function(instance, pCreateInfo, pAllocator, pDebugMessenger);
}

void vkDestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
{
	PFN_vkDestroyDebugUtilsMessengerEXT function = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));

	if (function != nullptr)
		return function(instance, debugMessenger, pAllocator);
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

SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface)
{
	SwapChainSupportDetails details;

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

VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, glacier::Window* window)
{
	if (capabilities.currentExtent.width != UINT32_MAX)
	{
		// If screen coordinates match pixels, return the current extent in screen coordinates.
		return capabilities.currentExtent;
	}
	else
	{
		// If screen coordinates don't match pixels, get the current extent in pixels from GLFW
		glm::uvec2 size = window->getFramebufferSize();

		VkExtent2D actualExtent = { static_cast<uint32_t>(size.x), static_cast<uint32_t>(size.y) };

		return actualExtent;
	}
}

bool isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface)
{
	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(device, &deviceProperties);

	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

	QueueFamilyIndices queueFamilyIndices = findQueueFamilies(device, surface);

	if (!queueFamilyIndices.isComplete())
		return false;

	SwapChainSupportDetails details = querySwapChainSupport(device, surface);
	if (!details.isAdequate())
		return false;

	return true;
}

glacier::Application::Application(const ApplicationInfo& info)
	: m_Info(info)
{
	g_Logger->info("Initializing application...");

	g_Logger->debug("Creating window...");
	m_Window = new glacier::Window(m_Info.windowInfo);

	/* Create Vulkan instance */
	g_Logger->debug("Creating Vulkan instance");

	VkApplicationInfo applicationInfo = {};
	applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	applicationInfo.pApplicationName = m_Info.name;
	applicationInfo.applicationVersion = VK_MAKE_VERSION(m_Info.major, m_Info.minor, m_Info.patch);
	applicationInfo.pEngineName = "Glacier Engine";
	applicationInfo.engineVersion = VK_MAKE_VERSION(0, 1, 0);
	applicationInfo.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo instanceCreateInfo = {};
	instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceCreateInfo.pApplicationInfo = &applicationInfo;

	// Extensions
	unsigned int glfwExtensionCount = 0;
	const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

#ifndef NDEBUG
	extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

	instanceCreateInfo.ppEnabledExtensionNames = extensions.data();
	instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());

	// Validation layers
#ifndef NDEBUG
	std::vector<const char*> layers;

	layers.push_back("VK_LAYER_KHRONOS_validation");

	instanceCreateInfo.ppEnabledLayerNames = layers.data();
	instanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(layers.size());
#else
	instanceCreateInfo.enabledLayerCount = 0;
#endif

#ifndef NDEBUG
	VkDebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo = {};
	debugMessengerCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	debugMessengerCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	debugMessengerCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	debugMessengerCreateInfo.pfnUserCallback = debugCallback;

	instanceCreateInfo.pNext = &debugMessengerCreateInfo;
#endif

	if (vkCreateInstance(&instanceCreateInfo, nullptr, reinterpret_cast<VkInstance*>(&m_VulkanInstance)) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create Vulkan instance");
	}

#ifndef NDEBUG
	if (vkCreateDebugUtilsMessengerEXT(static_cast<VkInstance>(m_VulkanInstance), &debugMessengerCreateInfo, nullptr, reinterpret_cast<VkDebugUtilsMessengerEXT*>(&m_DebugMessenger)) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create Vulkan debug messenger");
	}
#endif

	/* Create a window surface */
	g_Logger->debug("Creating window surface...");
	if (glfwCreateWindowSurface(static_cast<VkInstance>(m_VulkanInstance), static_cast<GLFWwindow*>(m_Window->m_Window), nullptr, reinterpret_cast<VkSurfaceKHR*>(&m_Surface)) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create window surface");
	}

	/* Pick a GPU */
	g_Logger->debug("Picking GPU...");
	unsigned int deviceCount = 0;
	vkEnumeratePhysicalDevices(static_cast<VkInstance>(m_VulkanInstance), &deviceCount, nullptr);

	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(static_cast<VkInstance>(m_VulkanInstance), &deviceCount, devices.data());

	m_PhysicalDevice = VK_NULL_HANDLE;

	for (VkPhysicalDevice device : devices)
	{
		if (!isDeviceSuitable(device, static_cast<VkSurfaceKHR>(m_Surface)))
			continue;

		VkPhysicalDeviceProperties deviceProperties;
		vkGetPhysicalDeviceProperties(device, &deviceProperties);

		g_Logger->info("Selected GPU: {}", deviceProperties.deviceName);
		g_Logger->info("Vulkan version: {}.{}.{}", VK_VERSION_MAJOR(deviceProperties.apiVersion), VK_VERSION_MINOR(deviceProperties.apiVersion), VK_VERSION_PATCH(deviceProperties.apiVersion));

		m_PhysicalDevice = device;
		break;
	}

	if (m_PhysicalDevice == VK_NULL_HANDLE)
	{
		throw std::runtime_error("Failed to find a suitable GPU");
	}

	/* Create a logical device */
	g_Logger->debug("Creating logical device...");

	QueueFamilyIndices queueFamilyIndices = findQueueFamilies(static_cast<VkPhysicalDevice>(m_PhysicalDevice), static_cast<VkSurfaceKHR>(m_Surface));

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

	std::set<unsigned int> uniqueQueueFamilies = { queueFamilyIndices.graphicsFamily.value(), queueFamilyIndices.presentationFamily.value() };

	for (unsigned int queueFamilyIndex : uniqueQueueFamilies)
	{
		VkDeviceQueueCreateInfo queueCreateInfo = {};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamilyIndex;
		queueCreateInfo.queueCount = 1;

		float priority = 1.0f;
		queueCreateInfo.pQueuePriorities = &priority;

		queueCreateInfos.push_back(queueCreateInfo);
	}

	VkPhysicalDeviceFeatures deviceFeatures = {};

	VkDeviceCreateInfo deviceCreateInfo = {};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
	deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	deviceCreateInfo.pEnabledFeatures = &deviceFeatures;

#ifndef NDEBUG
	deviceCreateInfo.ppEnabledLayerNames = layers.data();
	deviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(layers.size());
#else
	deviceCreateInfo.enabledLayerCount = 0;
#endif

	std::vector<const char*> deviceExtensions;
	deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

	deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
	deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());

	if (vkCreateDevice(static_cast<VkPhysicalDevice>(m_PhysicalDevice), &deviceCreateInfo, nullptr, reinterpret_cast<VkDevice*>(&m_Device)) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create logical device");
	}

	g_Logger->info("Application initialized.");
}

glacier::Application::~Application()
{
	g_Logger->info("Terminating application...");

	vkDestroyDevice(static_cast<VkDevice>(m_Device), nullptr);
	vkDestroySurfaceKHR(static_cast<VkInstance>(m_VulkanInstance), static_cast<VkSurfaceKHR>(m_Surface), nullptr);
	vkDestroyDebugUtilsMessengerEXT(static_cast<VkInstance>(m_VulkanInstance), static_cast<VkDebugUtilsMessengerEXT>(m_DebugMessenger), nullptr);
	vkDestroyInstance(static_cast<VkInstance>(m_VulkanInstance), nullptr);

	g_Logger->debug("Destroying window...");

	delete m_Window;

	g_Logger->info("Application terminated.");
}

void glacier::Application::run()
{
	/* Create a swap chain */
	g_Logger->debug("Creating swap chain");

	SwapChainSupportDetails details = querySwapChainSupport(static_cast<VkPhysicalDevice>(m_PhysicalDevice), static_cast<VkSurfaceKHR>(m_Surface));

	VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(details.formats);
	VkPresentModeKHR presentMode = choosePresentMode(details.presentModes, m_Info.vsync);
	VkExtent2D extent = chooseSwapExtent(details.capabilities, m_Window);

	unsigned int imageCount = details.capabilities.minImageCount + 1;
	if (details.capabilities.minImageCount > 0 && imageCount > details.capabilities.maxImageCount)
	{
		imageCount = details.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR swapchainCreateInfo = {};
	swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainCreateInfo.surface = static_cast<VkSurfaceKHR>(m_Surface);
	swapchainCreateInfo.minImageCount = imageCount;
	swapchainCreateInfo.imageFormat = surfaceFormat.format;
	swapchainCreateInfo.imageColorSpace = surfaceFormat.colorSpace;
	swapchainCreateInfo.imageExtent = extent;
	swapchainCreateInfo.imageArrayLayers = 1;

	// Images will be written to directly
	swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	QueueFamilyIndices queueFamilyIndices = findQueueFamilies(static_cast<VkPhysicalDevice>(m_PhysicalDevice), static_cast<VkSurfaceKHR>(m_Surface));

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

	VkSwapchainKHR swapchain;

	if (vkCreateSwapchainKHR(static_cast<VkDevice>(m_Device), &swapchainCreateInfo, nullptr, &swapchain) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create swapchain");
	}

	/* Create image views */
	g_Logger->debug("Creating image views...");

	VkFormat swapchainImageFormat = surfaceFormat.format;
	VkExtent2D swapchainExtent = extent;

	vkGetSwapchainImagesKHR(static_cast<VkDevice>(m_Device), swapchain, &imageCount, nullptr);

	std::vector<VkImage> swapchainImages(imageCount);
	vkGetSwapchainImagesKHR(static_cast<VkDevice>(m_Device), swapchain, &imageCount, swapchainImages.data());

	std::vector<VkImageView> imageViews(swapchainImages.size());
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

		if (vkCreateImageView(static_cast<VkDevice>(m_Device), &imageViewCreateInfo, nullptr, &(*(imageViews.begin() + i))) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create image view");
		}
	}

	/* Get device queues */
	//vkGetDeviceQueue(static_cast<VkDevice>(m_Device), queueFamilyIndices.graphicsFamily.value(), 0, reinterpret_cast<VkQueue*>(&m_GraphicsQueue));
	//vkGetDeviceQueue(static_cast<VkDevice>(m_Device), queueFamilyIndices.presentationFamily.value(), 0, reinterpret_cast<VkQueue*>(&m_PresentationQueue));

	/* Create render pass */
	g_Logger->debug("Creating render pass...");

	// Create color attachment (To clear the screen)
	VkAttachmentDescription colorAttachment = {};
	colorAttachment.format = swapchainImageFormat;
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

	VkRenderPass renderPass;

	if (vkCreateRenderPass(static_cast<VkDevice>(m_Device), &renderPassCreateInfo, nullptr, &renderPass) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create render pass");
	}

	/* Create pipeline */
	g_Logger->debug("Creating pipeline...");

	PipelineInfo pipeline = {};
	this->initialize(pipeline);

	std::vector<VkPipelineShaderStageCreateInfo> shaderStages;

	bool hasVertex = false, hasFragment = false;

	for (Shader* shader : pipeline.shaders)
	{
		VkPipelineShaderStageCreateInfo shaderCreateInfo = {};
		shaderCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;

		switch (shader->m_Type)
		{
		case ShaderType::Vertex:
			shaderCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;

			if (!hasVertex)
				hasVertex = true;
			else
				throw std::runtime_error("You can't have more than one vertex shader");

			break;
		case ShaderType::Fragment:
			shaderCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;

			if (!hasFragment)
				hasFragment = true;
			else
				throw std::runtime_error("You can't have more than one fragment shader");

			break;
		}

		shaderCreateInfo.module = static_cast<VkShaderModule>(shader->m_ShaderModule);
		shaderCreateInfo.pName = "main";

		shaderStages.push_back(shaderCreateInfo);
	}

	if (!hasVertex || !hasFragment)
		throw std::runtime_error("At least one vertex and fragment shader must exist");

	VkPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo = {};
	inputAssemblyCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssemblyCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssemblyCreateInfo.primitiveRestartEnable = VK_FALSE;

	VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo = {};
	vertexInputCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputCreateInfo.vertexBindingDescriptionCount = 0;
	vertexInputCreateInfo.pVertexBindingDescriptions = nullptr;
	vertexInputCreateInfo.vertexAttributeDescriptionCount = 0;
	vertexInputCreateInfo.pVertexAttributeDescriptions = nullptr;

	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;

	glm::uvec2 size = m_Window->getFramebufferSize();
	viewport.width = static_cast<float>(size.x);
	viewport.height = static_cast<float>(size.y);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor = {};
	scissor.offset = { 0, 0 };
	scissor.extent = { size.x, size.y };

	VkPipelineViewportStateCreateInfo viewportCreateInfo = {};
	viewportCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportCreateInfo.viewportCount = 1;
	viewportCreateInfo.pViewports = &viewport;
	viewportCreateInfo.scissorCount = 1;
	viewportCreateInfo.pScissors = &scissor;

	VkPipelineRasterizationStateCreateInfo rasterizerCreateInfo = {};
	rasterizerCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizerCreateInfo.depthClampEnable = VK_FALSE;
	rasterizerCreateInfo.rasterizerDiscardEnable = VK_FALSE;
	rasterizerCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizerCreateInfo.lineWidth = 1.0f;

	// Disable face culling
	rasterizerCreateInfo.cullMode = VK_CULL_MODE_NONE; // VK_CULL_MODE_BACK_BIT

	rasterizerCreateInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizerCreateInfo.depthBiasEnable = VK_FALSE;
	rasterizerCreateInfo.depthBiasConstantFactor = 0.0f;
	rasterizerCreateInfo.depthBiasClamp = 0.0f;
	rasterizerCreateInfo.depthBiasSlopeFactor = 0.0f;

	VkPipelineMultisampleStateCreateInfo multisampleCreateInfo = {};
	multisampleCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampleCreateInfo.sampleShadingEnable = VK_FALSE;
	multisampleCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampleCreateInfo.minSampleShading = 1.0f;
	multisampleCreateInfo.pSampleMask = nullptr;
	multisampleCreateInfo.alphaToCoverageEnable = VK_FALSE;
	multisampleCreateInfo.alphaToOneEnable = VK_FALSE;

	VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;

	VkPipelineColorBlendStateCreateInfo colorBlendCreateInfo = {};
	colorBlendCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlendCreateInfo.logicOpEnable = VK_FALSE;
	colorBlendCreateInfo.logicOp = VK_LOGIC_OP_COPY;
	colorBlendCreateInfo.attachmentCount = 1;
	colorBlendCreateInfo.pAttachments = &colorBlendAttachment;
	colorBlendCreateInfo.blendConstants[0] = 0.0f;
	colorBlendCreateInfo.blendConstants[1] = 0.0f;
	colorBlendCreateInfo.blendConstants[2] = 0.0f;
	colorBlendCreateInfo.blendConstants[3] = 0.0f;

	/* Dynamic state
	VkDynamicState dynamicStates[] = {
		VK_DYNAMIC_STATE_VIEWPORT
	};

	VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo = {};
	dynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicStateCreateInfo.dynamicStateCount = 1;
	dynamicStateCreateInfo.pDynamicStates = dynamicStates;
	*/

	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
	pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCreateInfo.setLayoutCount = 0;
	pipelineLayoutCreateInfo.pSetLayouts = nullptr;
	pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
	pipelineLayoutCreateInfo.pPushConstantRanges = nullptr;

	VkPipelineLayout pipelineLayout;

	if (vkCreatePipelineLayout(static_cast<VkDevice>(m_Device), &pipelineLayoutCreateInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create pipeline layout");
	}

	/* Create graphics pipeline */
	VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo = {};
	graphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;

	// Shader stages
	graphicsPipelineCreateInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
	graphicsPipelineCreateInfo.pStages = shaderStages.data();

	graphicsPipelineCreateInfo.pVertexInputState = &vertexInputCreateInfo;
	graphicsPipelineCreateInfo.pInputAssemblyState = &inputAssemblyCreateInfo;
	graphicsPipelineCreateInfo.pViewportState = &viewportCreateInfo;
	graphicsPipelineCreateInfo.pRasterizationState = &rasterizerCreateInfo;
	graphicsPipelineCreateInfo.pMultisampleState = &multisampleCreateInfo;
	graphicsPipelineCreateInfo.pDepthStencilState = nullptr;
	graphicsPipelineCreateInfo.pColorBlendState = &colorBlendCreateInfo;

	// TODO: Implement dynamic state (To change viewport size)
	graphicsPipelineCreateInfo.pDynamicState = nullptr;

	graphicsPipelineCreateInfo.layout = pipelineLayout;

	graphicsPipelineCreateInfo.renderPass = renderPass;
	graphicsPipelineCreateInfo.subpass = 0;

	graphicsPipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
	graphicsPipelineCreateInfo.basePipelineIndex = -1;

	VkPipeline graphicsPipeline;

	if (vkCreateGraphicsPipelines(static_cast<VkDevice>(m_Device), VK_NULL_HANDLE, 1, &graphicsPipelineCreateInfo, nullptr, &graphicsPipeline) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create graphics pipeline");
	}

	/* Create framebuffers */
	g_Logger->debug("Creating framebuffers...");

	std::vector<VkFramebuffer> framebuffers(imageViews.size());

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

		if (vkCreateFramebuffer(static_cast<VkDevice>(m_Device), &framebufferCreateInfo, nullptr, &framebuffers[i]) != VK_SUCCESS)
		{
			throw std::runtime_error(fmt::format("Failed to create framebuffer {}", i));
		}
	}

	// Create command pool
	VkCommandPoolCreateInfo commandPoolCreateInfo = {};
	commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolCreateInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
	commandPoolCreateInfo.flags = 0;

	VkCommandPool commandPool;

	if (vkCreateCommandPool(static_cast<VkDevice>(m_Device), &commandPoolCreateInfo, nullptr, &commandPool) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create command pool");
	}

	// Create command buffers
	std::vector<VkCommandBuffer> commandBuffers(framebuffers.size());

	VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
	commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferAllocateInfo.commandPool = commandPool;
	commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	commandBufferAllocateInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

	if (vkAllocateCommandBuffers(static_cast<VkDevice>(m_Device), &commandBufferAllocateInfo, commandBuffers.data()) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to allocate command buffers");
	}

	/* RENDER */
	for (size_t i = 0; i < commandBuffers.size(); i++)
	{
		VkCommandBufferBeginInfo commandBufferBeginInfo = {};
		commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		commandBufferBeginInfo.flags = 0;
		commandBufferBeginInfo.pInheritanceInfo = nullptr;

		if (vkBeginCommandBuffer(commandBuffers[i], &commandBufferBeginInfo) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to begin command buffer");
		}

		// Begin render pass
		VkRenderPassBeginInfo renderPassBeginInfo = {};
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.renderPass = renderPass;
		renderPassBeginInfo.framebuffer = framebuffers[i];

		renderPassBeginInfo.renderArea.offset = { 0, 0 };
		renderPassBeginInfo.renderArea.extent = swapchainExtent;

		VkClearValue clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
		renderPassBeginInfo.clearValueCount = 1;
		renderPassBeginInfo.pClearValues = &clearColor;

		vkCmdBeginRenderPass(commandBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
		vkCmdDraw(commandBuffers[i], 3, 1, 0, 0);

		vkCmdEndRenderPass(commandBuffers[i]);

		if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to end command buffer");
		}
	}

	/* Start game loop */
	g_Logger->debug("Starting game loop...");

	double lastTime = glfwGetTime();
	while (m_Window->isOpen())
	{
		double deltaTime = glfwGetTime() - lastTime;
		lastTime = glfwGetTime();

		update(deltaTime);
		render();

		glfwPollEvents();
	}

	g_Logger->debug("Game loop stopped.");


	/* Destroy renderer */
	g_Logger->debug("Destroying renderer...");

	vkDestroyCommandPool(static_cast<VkDevice>(m_Device), commandPool, nullptr);

	for (const VkFramebuffer& framebuffer : framebuffers)
	{
		vkDestroyFramebuffer(static_cast<VkDevice>(m_Device), framebuffer, nullptr);
	}

	vkDestroyPipeline(static_cast<VkDevice>(m_Device), graphicsPipeline, nullptr);
	vkDestroyPipelineLayout(static_cast<VkDevice>(m_Device), pipelineLayout, nullptr);
	vkDestroyRenderPass(static_cast<VkDevice>(m_Device), renderPass, nullptr);

	for (const VkImageView& imageView : imageViews)
	{
		vkDestroyImageView(static_cast<VkDevice>(m_Device), imageView, nullptr);
	}

	vkDestroySwapchainKHR(static_cast<VkDevice>(m_Device), swapchain, nullptr);
}

void glacier::Application::stop()
{
	m_Window->close();
}

#pragma warning(pop)

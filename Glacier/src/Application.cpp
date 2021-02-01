#include "Application.hpp"

#include <vector>
#include <iostream>
#include <set>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

std::shared_ptr<spdlog::logger> createLogger(const char* name)
{
	std::shared_ptr<spdlog::logger> logger = spdlog::stdout_color_mt(name);
	logger->set_level(spdlog::level::debug);
	return logger;
}

std::shared_ptr<spdlog::logger> glacier::Application::s_Logger = createLogger("console");

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
	}

	if (level == spdlog::level::info)
		return VK_FALSE;

	glacier::Application::s_Logger->log(level, "[Vulkan] {}", pCallbackData->pMessage);
	
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

VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* windowHandle)
{
	if (capabilities.currentExtent.width != UINT32_MAX)
	{
		// If screen coordinates match pixels, return the current extent in screen coordinates.
		return capabilities.currentExtent;
	}
	else
	{
		// If screen coordinates don't match pixels, get the current extent in pixels from GLFW
		int width, height;
		glfwGetFramebufferSize(windowHandle, &width, &height);

		VkExtent2D actualExtent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };

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
{
	s_Logger->info("Initializing application...");

	s_Logger->debug("Creating window...");
	m_Window = new glacier::Window(info.windowInfo);

	/* Create Vulkan instance */
	s_Logger->debug("Creating Vulkan instance");

	VkApplicationInfo applicationInfo = {};
	applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	applicationInfo.pApplicationName = info.name;
	applicationInfo.applicationVersion = VK_MAKE_VERSION(info.major, info.minor, info.patch);
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
	s_Logger->debug("Creating window surface...");
	if (glfwCreateWindowSurface(static_cast<VkInstance>(m_VulkanInstance), static_cast<GLFWwindow*>(m_Window->m_Window), nullptr, reinterpret_cast<VkSurfaceKHR*>(&m_Surface)) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create window surface");
	}

	/* Pick a GPU */
	s_Logger->debug("Picking GPU...");
	unsigned int deviceCount = 0;
	vkEnumeratePhysicalDevices(static_cast<VkInstance>(m_VulkanInstance), &deviceCount, nullptr);

	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(static_cast<VkInstance>(m_VulkanInstance), &deviceCount, devices.data());

	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

	for (VkPhysicalDevice device : devices)
	{
		if (!isDeviceSuitable(device, static_cast<VkSurfaceKHR>(m_Surface)))
			continue;

		VkPhysicalDeviceProperties deviceProperties;
		vkGetPhysicalDeviceProperties(device, &deviceProperties);

		s_Logger->info("Selected GPU: {}", deviceProperties.deviceName);
		s_Logger->info("Vulkan version: {}.{}.{}", VK_VERSION_MAJOR(deviceProperties.apiVersion), VK_VERSION_MINOR(deviceProperties.apiVersion), VK_VERSION_PATCH(deviceProperties.apiVersion));

		physicalDevice = device;
		break;
	}

	if (physicalDevice == VK_NULL_HANDLE)
	{
		throw std::runtime_error("Failed to find a suitable GPU");
	}

	/* Create a logical device */
	s_Logger->debug("Creating logical device...");

	QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice, static_cast<VkSurfaceKHR>(m_Surface));

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

	if (vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, reinterpret_cast<VkDevice*>(&m_Device)) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create logical device");
	}

	/* Create a swap chain */
	s_Logger->debug("Creating swap chain");

	SwapChainSupportDetails details = querySwapChainSupport(physicalDevice, static_cast<VkSurfaceKHR>(m_Surface));
	
	VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(details.formats);
	VkPresentModeKHR presentMode = choosePresentMode(details.presentModes, info.vsync);
	VkExtent2D extent = chooseSwapExtent(details.capabilities, static_cast<GLFWwindow*>(m_Window->m_Window));

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

	if (vkCreateSwapchainKHR(static_cast<VkDevice>(m_Device), &swapchainCreateInfo, nullptr, reinterpret_cast<VkSwapchainKHR*>(&m_Swapchain)) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create swapchain");
	}

	// Get the swapchain images
	vkGetSwapchainImagesKHR(static_cast<VkDevice>(m_Device), static_cast<VkSwapchainKHR>(m_Swapchain), &imageCount, nullptr);
	
	std::vector<VkImage> swapchainImages(imageCount);
	vkGetSwapchainImagesKHR(static_cast<VkDevice>(m_Device), static_cast<VkSwapchainKHR>(m_Swapchain), &imageCount, swapchainImages.data());

	/* Get device queues */
	vkGetDeviceQueue(static_cast<VkDevice>(m_Device), queueFamilyIndices.graphicsFamily.value(), 0, reinterpret_cast<VkQueue*>(&m_GraphicsQueue));
	vkGetDeviceQueue(static_cast<VkDevice>(m_Device), queueFamilyIndices.presentationFamily.value(), 0, reinterpret_cast<VkQueue*>(&m_PresentationQueue));

	// https://vulkan-tutorial.com/en/Drawing_a_triangle/Presentation/Swap_chain
}

glacier::Application::~Application()
{
	s_Logger->info("Terminating application...");

	s_Logger->debug("Destroying Vulkan instance...");

	vkDestroySwapchainKHR(static_cast<VkDevice>(m_Device), static_cast<VkSwapchainKHR>(m_Swapchain), nullptr);
	vkDestroyDevice(static_cast<VkDevice>(m_Device), nullptr);
	vkDestroySurfaceKHR(static_cast<VkInstance>(m_VulkanInstance), static_cast<VkSurfaceKHR>(m_Surface), nullptr);
	vkDestroyDebugUtilsMessengerEXT(static_cast<VkInstance>(m_VulkanInstance), static_cast<VkDebugUtilsMessengerEXT>(m_DebugMessenger), nullptr);
	vkDestroyInstance(static_cast<VkInstance>(m_VulkanInstance), nullptr);

	s_Logger->debug("Destroying window...");
	delete m_Window;
}

void glacier::Application::run()
{
	while (m_Window->isOpen())
	{
		glfwPollEvents();
	}
}

#include "Application.hpp"

#include <vector>
#include <iostream>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

std::shared_ptr<spdlog::logger> glacier::Application::s_Logger = spdlog::stdout_color_mt("console");

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

glacier::Application::Application(const ApplicationInfo& info)
{
	m_Window = new glacier::Window(info.windowInfo);

	/* Create Vulkan instance */
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

	if (vkCreateInstance(&instanceCreateInfo, nullptr, reinterpret_cast<VkInstance*>(&m_VulkanInstance)) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create Vulkan instance");
	}

#ifndef NDEBUG
	VkDebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo = {};
	debugMessengerCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	debugMessengerCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	debugMessengerCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	debugMessengerCreateInfo.pfnUserCallback = debugCallback;

	// instanceCreateInfo.pNext = &debugMessengerCreateInfo;

	if (vkCreateDebugUtilsMessengerEXT(static_cast<VkInstance>(m_VulkanInstance), &debugMessengerCreateInfo, nullptr, reinterpret_cast<VkDebugUtilsMessengerEXT*>(&m_DebugMessenger)) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create Vulkan debug messenger");
	}
#endif
}

glacier::Application::~Application()
{
	//vkDestroyDebugUtilsMessengerEXT(static_cast<VkInstance>(m_VulkanInstance), static_cast<VkDebugUtilsMessengerEXT>(m_DebugMessenger), nullptr);
	vkDestroyInstance(static_cast<VkInstance>(m_VulkanInstance), nullptr);

	delete m_Window;
}

void glacier::Application::run()
{
	while (m_Window->isOpen())
	{
		glfwPollEvents();
	}
}

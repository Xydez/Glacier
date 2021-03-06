#pragma warning(push)
#pragma warning(disable: 26812)

#include "Application.hpp"
#include "VertexBuffer.hpp"
#include "Renderer.hpp"
#include "internal/utility.hpp"

#include <vector>
#include <iostream>
#include <set>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

constexpr unsigned int MAX_BUFFERED_FRAMES = 2;

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

bool isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface)
{
	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(device, &deviceProperties);

	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

	QueueFamilyIndices queueFamilyIndices = findQueueFamilies(device, surface);

	if (!queueFamilyIndices.isComplete())
		return false;

	SwapchainSupportDetails details = querySwapchainSupport(device, surface);
	if (!details.isAdequate())
		return false;

	return true;
}

glacier::Application::Application(const ApplicationInfo& info)
	: m_Info(info), m_FramebufferResized(false), m_Renderer(nullptr)
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
	if (glfwCreateWindowSurface(static_cast<VkInstance>(m_VulkanInstance), static_cast<GLFWwindow*>(m_Window->m_Handle), nullptr, reinterpret_cast<VkSurfaceKHR*>(&m_Surface)) != VK_SUCCESS)
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
	VkResult result = VK_SUCCESS;

	initialize();

	m_Renderer = new Renderer(this);
	initializeRenderer(m_Renderer);

	/* Create semaphores */
	std::vector<VkSemaphore> imageAvailableSemaphores(MAX_BUFFERED_FRAMES);
	std::vector<VkSemaphore> renderFinishedSemaphores(MAX_BUFFERED_FRAMES);
	std::vector<VkFence> bufferedFences(MAX_BUFFERED_FRAMES);
	std::vector<VkFence> bufferedImageFences(m_Renderer->m_Images.size(), VK_NULL_HANDLE);

	VkSemaphoreCreateInfo semaphoreCreateInfo = {};
	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceCreateInfo = {};
	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (size_t i = 0; i < MAX_BUFFERED_FRAMES; i++)
	{
		result = vkCreateSemaphore(static_cast<VkDevice>(m_Device), &semaphoreCreateInfo, nullptr, &(imageAvailableSemaphores[i]));
		if (result != VK_SUCCESS)
		{
			throw std::runtime_error(fmt::format("Failed to create image available semaphore (Returned {})", result));
		}

		result = vkCreateSemaphore(static_cast<VkDevice>(m_Device), &semaphoreCreateInfo, nullptr, &(renderFinishedSemaphores[i]));
		if (result != VK_SUCCESS)
		{
			throw std::runtime_error(fmt::format("Failed to create render finished semaphore (Returned {})", result));
		}

		result = vkCreateFence(static_cast<VkDevice>(m_Device), &fenceCreateInfo, nullptr, &(bufferedFences[i]));
		if (result != VK_SUCCESS)
		{
			throw std::runtime_error(fmt::format("Failed to create fence (Returned {})", result));
		}
	}

	/* Start game loop */
	g_Logger->debug("Starting game loop...");

	size_t currentFrame = 0;
	bool suboptimal_flag = false;

	glfwSetWindowUserPointer(static_cast<GLFWwindow*>(m_Window->m_Handle), &m_FramebufferResized);
	glfwSetFramebufferSizeCallback(static_cast<GLFWwindow*>(m_Window->m_Handle), [](GLFWwindow* window, int width, int height) -> void
		{
			bool* framebufferResized = reinterpret_cast<bool*>(glfwGetWindowUserPointer(window));
			*framebufferResized = true;
		});

	double lastTime = glfwGetTime();
	while (m_Window->isOpen())
	{
		double deltaTime = glfwGetTime() - lastTime;
		lastTime = glfwGetTime();

		update(deltaTime);

		/* Draw frame */
		// Wait until the next frame should be drawn
		vkWaitForFences(static_cast<VkDevice>(m_Device), 1, &(bufferedFences[currentFrame]), VK_TRUE, UINT64_MAX);

		uint32_t imageIndex;
		result = vkAcquireNextImageKHR(static_cast<VkDevice>(m_Device), static_cast<VkSwapchainKHR>(m_Renderer->m_Swapchain), UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

		if (result == VK_ERROR_OUT_OF_DATE_KHR || m_FramebufferResized)
		{
			if (m_FramebufferResized)
			{
				glacier::g_Logger->debug("Framebuffer was resized");
				m_FramebufferResized = false;
			}

			/* Check if the window was minimized */
			glm::uvec2 size = m_Window->getFramebufferSize();
			if (size.x == 0 || size.y == 0)
			{
				glacier::g_Logger->trace("Window is minimized");

				while (size.x == 0 || size.y == 0)
				{
					size = m_Window->getFramebufferSize();
					glfwWaitEvents();
				}

				glacier::g_Logger->trace("Window is no longer minimized");
			}

			/* Recreate the swapchain */
			glacier::g_Logger->trace("Swapchain is outdated.");

			terminateRenderer(m_Renderer);

			delete m_Renderer;
			m_Renderer = new Renderer(this);

			initializeRenderer(m_Renderer);

			/* Recreate semaphores */
			for (size_t i = 0; i < MAX_BUFFERED_FRAMES; i++)
			{
				vkDestroySemaphore(static_cast<VkDevice>(m_Device), imageAvailableSemaphores[i], nullptr);

				result = vkCreateSemaphore(static_cast<VkDevice>(m_Device), &semaphoreCreateInfo, nullptr, &(imageAvailableSemaphores[i]));
				if (result != VK_SUCCESS)
				{
					throw std::runtime_error(fmt::format("Failed to create image available semaphore (Returned {})", result));
				}

				vkDestroySemaphore(static_cast<VkDevice>(m_Device), renderFinishedSemaphores[i], nullptr);

				result = vkCreateSemaphore(static_cast<VkDevice>(m_Device), &semaphoreCreateInfo, nullptr, &(renderFinishedSemaphores[i]));
				if (result != VK_SUCCESS)
				{
					throw std::runtime_error(fmt::format("Failed to create render finished semaphore (Returned {})", result));
				}
			}

			continue;
		}
		else if (result == VK_SUBOPTIMAL_KHR)
		{
			if (!suboptimal_flag)
			{
				g_Logger->warn("Swapchain is suboptimal.");
				suboptimal_flag = true;
			}
		}
		else if (result == VK_SUCCESS)
		{
			suboptimal_flag = false;
		}
		else
		{
			throw std::runtime_error(fmt::format("Failed to acquire swapchain image (Returned {})", result));
		}

		if (bufferedImageFences[imageIndex] != VK_NULL_HANDLE)
		{
			vkWaitForFences(static_cast<VkDevice>(m_Device), 1, &(bufferedImageFences[imageIndex]), VK_TRUE, UINT64_MAX);
		}

		bufferedImageFences[imageIndex] = bufferedFences[currentFrame];

		// Render the frame <ERROR HERE>
		render(m_Renderer);

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame] };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;

		// Renderer needs to have a pipeline bound
		if (m_Renderer->m_CommandBuffers.empty())
			throw std::runtime_error("Renderer has no pipeline bound");

		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = reinterpret_cast<VkCommandBuffer*>(&m_Renderer->m_CommandBuffers[imageIndex]); //&commandBuffers[imageIndex];

		VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame] };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		vkResetFences(static_cast<VkDevice>(m_Device), 1, &(bufferedFences[currentFrame]));
		result = vkQueueSubmit(static_cast<VkQueue>(m_Renderer->m_GraphicsQueue), 1, &submitInfo, bufferedFences[currentFrame]);
		if (result != VK_SUCCESS)
		{
			throw std::runtime_error(fmt::format("Failed to submit draw command buffer (Returned {})", result));
		}

		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;

		VkSwapchainKHR swapchains[] = { static_cast<VkSwapchainKHR>(m_Renderer->m_Swapchain) };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapchains; // TODO: &swapchain ?
		presentInfo.pImageIndices = &imageIndex;

		result = vkQueuePresentKHR(static_cast<VkQueue>(m_Renderer->m_PresentationQueue), &presentInfo);
		if (result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			/* Check if the window was minimized */
			glm::uvec2 size = m_Window->getFramebufferSize();
			if (size.x == 0 || size.y == 0)
			{
				glacier::g_Logger->trace("Window is minimized");

				while (size.x == 0 || size.y == 0)
				{
					size = m_Window->getFramebufferSize();
					glfwWaitEvents();
				}

				glacier::g_Logger->trace("Window is no longer minimized");
			}

			/* Recreate the swapchain */
			glacier::g_Logger->trace("Swapchain is outdated.");

			terminateRenderer(m_Renderer);

			delete m_Renderer;
			m_Renderer = new Renderer(this);

			initializeRenderer(m_Renderer);

			/* Recreate semaphores */
			for (size_t i = 0; i < MAX_BUFFERED_FRAMES; i++)
			{
				vkDestroySemaphore(static_cast<VkDevice>(m_Device), imageAvailableSemaphores[i], nullptr);

				result = vkCreateSemaphore(static_cast<VkDevice>(m_Device), &semaphoreCreateInfo, nullptr, &(imageAvailableSemaphores[i]));
				if (result != VK_SUCCESS)
				{
					throw std::runtime_error(fmt::format("Failed to create image available semaphore (Returned {})", result));
				}

				vkDestroySemaphore(static_cast<VkDevice>(m_Device), renderFinishedSemaphores[i], nullptr);

				result = vkCreateSemaphore(static_cast<VkDevice>(m_Device), &semaphoreCreateInfo, nullptr, &(renderFinishedSemaphores[i]));
				if (result != VK_SUCCESS)
				{
					throw std::runtime_error(fmt::format("Failed to create render finished semaphore (Returned {})", result));
				}
			}

			continue;
		}
		else if (result != VK_SUCCESS)
		{
			throw std::runtime_error(fmt::format("Failed to present queue (Returned {})", result));
		}

		currentFrame = (currentFrame + 1) % MAX_BUFFERED_FRAMES;

		glfwPollEvents();
	}

	g_Logger->debug("Game loop stopped.");

	// Wait for m_Device to be finished
	vkDeviceWaitIdle(static_cast<VkDevice>(m_Device));

	/* Destroy renderer */
	g_Logger->debug("Destroying renderer...");

	// Destroy this shit last
	for (size_t i = 0; i < MAX_BUFFERED_FRAMES; i++)
	{
		vkDestroySemaphore(static_cast<VkDevice>(m_Device), imageAvailableSemaphores[i], nullptr);
		vkDestroySemaphore(static_cast<VkDevice>(m_Device), renderFinishedSemaphores[i], nullptr);
		vkDestroyFence(static_cast<VkDevice>(m_Device), bufferedFences[i], nullptr);
	}

	terminateRenderer(m_Renderer);
	delete m_Renderer;

	terminate();
}

void glacier::Application::stop()
{
	m_Window->close();
}

#pragma warning(pop)

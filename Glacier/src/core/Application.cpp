#pragma warning(push)
#pragma warning(disable: 26812)

#include "core/Application.hpp"
#include "graphics/VertexBuffer.hpp"
#include "graphics/Renderer.hpp"
#include "internal/utility.hpp"

#include <vector>
#include <iostream>
#include <set>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

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
	: m_Info(info), m_Renderer(nullptr)
{
	VkResult result;

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

	result = vkCreateInstance(&instanceCreateInfo, nullptr, reinterpret_cast<VkInstance*>(&m_VulkanInstance));
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error(fmt::format("Failed to create Vulkan instance (Returned {})", result));
	}

#ifndef NDEBUG
	result = vkCreateDebugUtilsMessengerEXT(static_cast<VkInstance>(m_VulkanInstance), &debugMessengerCreateInfo, nullptr, reinterpret_cast<VkDebugUtilsMessengerEXT*>(&m_DebugMessenger));
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error(fmt::format("Failed to create Vulkan debug messenger (Returned {})", result));
	}
#endif

	/* Create a window surface */
	g_Logger->debug("Creating window surface...");
	result = glfwCreateWindowSurface(static_cast<VkInstance>(m_VulkanInstance), static_cast<GLFWwindow*>(m_Window->m_Handle), nullptr, reinterpret_cast<VkSurfaceKHR*>(&m_Surface));
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error(fmt::format("Failed to create window surface (Returned {})", result));
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

	result = vkCreateDevice(static_cast<VkPhysicalDevice>(m_PhysicalDevice), &deviceCreateInfo, nullptr, reinterpret_cast<VkDevice*>(&m_Device));
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error(fmt::format("Failed to create logical device (Returned {})", result));
	}

	/* Create uniform buffer layout */
	VkDescriptorSetLayoutBinding layoutBinding;
	layoutBinding.binding = 0;
	layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	layoutBinding.descriptorCount = 1;
	layoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT; // TODO: Maybe we need to know this?
	layoutBinding.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutCreateInfo layoutCreateInfo = {};
	layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutCreateInfo.bindingCount = 1;
	layoutCreateInfo.pBindings = &layoutBinding;

	result = vkCreateDescriptorSetLayout(static_cast<VkDevice>(m_Device), &layoutCreateInfo, nullptr, reinterpret_cast<VkDescriptorSetLayout*>(&m_UniformBufferLayout));
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error(fmt::format("Failed to create uniform buffer layout descriptor (Returned {})", result));
	}

	g_Logger->info("Application initialized.");
}

glacier::Application::~Application()
{
	g_Logger->info("Terminating application...");

	vkDestroyDescriptorSetLayout(static_cast<VkDevice>(m_Device), static_cast<VkDescriptorSetLayout>(m_UniformBufferLayout), nullptr);

	vkDestroyDevice(static_cast<VkDevice>(m_Device), nullptr);
	vkDestroySurfaceKHR(static_cast<VkInstance>(m_VulkanInstance), static_cast<VkSurfaceKHR>(m_Surface), nullptr);
	vkDestroyDebugUtilsMessengerEXT(static_cast<VkInstance>(m_VulkanInstance), static_cast<VkDebugUtilsMessengerEXT>(m_DebugMessenger), nullptr);
	vkDestroyInstance(static_cast<VkInstance>(m_VulkanInstance), nullptr);

	g_Logger->debug("Destroying window...");

	delete m_Window;

	g_Logger->info("Application terminated.");
}

void app_render()
{
	
}

void glacier::Application::run()
{
	VkResult result = VK_SUCCESS;

	m_Renderer = new Renderer(this);
	initialize(m_Renderer);

	unsigned int bufferedFrameCount = m_Renderer->m_Images.size();

	/* Create semaphores */
	imageAvailableSemaphores.resize(bufferedFrameCount);
	renderFinishedSemaphores.resize(bufferedFrameCount);
	bufferedFences.resize(bufferedFrameCount);
	bufferedImageFences.resize(bufferedFrameCount);
	
	for (void*& ptr : bufferedImageFences)
		ptr = VK_NULL_HANDLE;

	VkSemaphoreCreateInfo semaphoreCreateInfo = {};
	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceCreateInfo = {};
	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (size_t i = 0; i < bufferedFrameCount; i++)
	{
		result = vkCreateSemaphore(static_cast<VkDevice>(m_Device), &semaphoreCreateInfo, nullptr, reinterpret_cast<VkSemaphore*>(&(imageAvailableSemaphores[i])));
		if (result != VK_SUCCESS)
		{
			throw std::runtime_error(fmt::format("Failed to create image available semaphore (Returned {})", result));
		}

		result = vkCreateSemaphore(static_cast<VkDevice>(m_Device), &semaphoreCreateInfo, nullptr, reinterpret_cast<VkSemaphore*>(&(renderFinishedSemaphores[i])));
		if (result != VK_SUCCESS)
		{
			throw std::runtime_error(fmt::format("Failed to create render finished semaphore (Returned {})", result));
		}

		result = vkCreateFence(static_cast<VkDevice>(m_Device), &fenceCreateInfo, nullptr, reinterpret_cast<VkFence*>(&(bufferedFences[i])));
		if (result != VK_SUCCESS)
		{
			throw std::runtime_error(fmt::format("Failed to create fence (Returned {})", result));
		}
	}

	/* Start game loop */
	g_Logger->debug("Starting game loop...");

	bool suboptimal_flag = false;

	glfwSetWindowUserPointer(static_cast<GLFWwindow*>(m_Window->m_Handle), this);
	glfwSetFramebufferSizeCallback(static_cast<GLFWwindow*>(m_Window->m_Handle), framebufferSizeCallback);

	double lastTime = glfwGetTime();
	while (m_Window->isOpen())
	{
		double deltaTime = glfwGetTime() - lastTime;
		lastTime = glfwGetTime();

		/* +================+ */
		/* |     Update     | */
		/* +================+ */
		update(deltaTime);

		/* +================+ */
		/* |     Render     | */
		/* +================+ */
		// Wait until the next frame should be drawn
		vkWaitForFences(static_cast<VkDevice>(m_Device), 1, reinterpret_cast<VkFence*>(&(bufferedFences[m_CurrentFrame])), VK_TRUE, UINT64_MAX);

		// Acquire a swapchain image
		uint32_t imageIndex;
		result = vkAcquireNextImageKHR(static_cast<VkDevice>(m_Device), static_cast<VkSwapchainKHR>(m_Renderer->m_Swapchain), UINT64_MAX, static_cast<VkSemaphore>(imageAvailableSemaphores[m_CurrentFrame]), VK_NULL_HANDLE, &imageIndex);

		// Check if the swapchain is valid
		if (result == VK_ERROR_OUT_OF_DATE_KHR) // || m_Resized
		{
			//if (m_Resized)
			//{
			//	glacier::g_Logger->debug("Framebuffer was resized");
			//	m_Resized = false;
			//}
			//else
			//{
			//	glacier::g_Logger->warn("Swapchain out of date");
			//}

			glacier::g_Logger->warn("Swapchain out of date");

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

			m_Renderer->destroy();
			m_Renderer->create();

			/* Recreate semaphores */
			for (size_t i = 0; i < bufferedFrameCount; i++)
			{
				vkDestroySemaphore(static_cast<VkDevice>(m_Device), static_cast<VkSemaphore>(imageAvailableSemaphores[i]), nullptr);

				result = vkCreateSemaphore(static_cast<VkDevice>(m_Device), &semaphoreCreateInfo, nullptr, reinterpret_cast<VkSemaphore*>(&(imageAvailableSemaphores[i])));
				if (result != VK_SUCCESS)
				{
					throw std::runtime_error(fmt::format("Failed to create image available semaphore (Returned {})", result));
				}

				vkDestroySemaphore(static_cast<VkDevice>(m_Device), static_cast<VkSemaphore>(renderFinishedSemaphores[i]), nullptr);

				result = vkCreateSemaphore(static_cast<VkDevice>(m_Device), &semaphoreCreateInfo, nullptr, reinterpret_cast<VkSemaphore*>(&(renderFinishedSemaphores[i])));
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

		// Wait until this frame can be rendered
		if (bufferedImageFences[imageIndex] != VK_NULL_HANDLE)
		{
			vkWaitForFences(static_cast<VkDevice>(m_Device), 1, reinterpret_cast<VkFence*>(&(bufferedImageFences[imageIndex])), VK_TRUE, UINT64_MAX);
		}

		bufferedImageFences[imageIndex] = bufferedFences[m_CurrentFrame];

		// Render the frame
		render(m_Renderer);

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkSemaphore waitSemaphores[] = { static_cast<VkSemaphore>(imageAvailableSemaphores[m_CurrentFrame]) };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;

		// Renderer needs to have a pipeline bound
		if (m_Renderer->m_CommandBuffers.empty())
		{
			g_Logger->error("Renderer has no pipeline bound");
			continue;
		}

		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = reinterpret_cast<VkCommandBuffer*>(&m_Renderer->m_CommandBuffers[imageIndex]); //&commandBuffers[imageIndex];

		VkSemaphore signalSemaphores[] = { static_cast<VkSemaphore>(renderFinishedSemaphores[m_CurrentFrame]) };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		vkResetFences(static_cast<VkDevice>(m_Device), 1, reinterpret_cast<VkFence*>(&(bufferedFences[m_CurrentFrame])));
		result = vkQueueSubmit(static_cast<VkQueue>(m_Renderer->m_GraphicsQueue), 1, &submitInfo, static_cast<VkFence>(bufferedFences[m_CurrentFrame]));
		if (result != VK_SUCCESS)
		{
			throw std::runtime_error(fmt::format("Failed to submit draw command buffer (Returned {})", result));
		}

		// Present the frame
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

			m_Renderer->destroy();
			m_Renderer->create();

			/* Recreate semaphores */
			for (size_t i = 0; i < bufferedFrameCount; i++)
			{
				vkDestroySemaphore(static_cast<VkDevice>(m_Device), static_cast<VkSemaphore>(imageAvailableSemaphores[i]), nullptr);

				result = vkCreateSemaphore(static_cast<VkDevice>(m_Device), &semaphoreCreateInfo, nullptr, reinterpret_cast<VkSemaphore*>(&(imageAvailableSemaphores[i])));
				if (result != VK_SUCCESS)
				{
					throw std::runtime_error(fmt::format("Failed to create image available semaphore (Returned {})", result));
				}

				vkDestroySemaphore(static_cast<VkDevice>(m_Device), static_cast<VkSemaphore>(renderFinishedSemaphores[i]), nullptr);

				result = vkCreateSemaphore(static_cast<VkDevice>(m_Device), &semaphoreCreateInfo, nullptr, reinterpret_cast<VkSemaphore*>(&(renderFinishedSemaphores[i])));
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

		m_CurrentFrame = (m_CurrentFrame + 1) % bufferedFrameCount;

		glfwPollEvents();
	}

	g_Logger->debug("Game loop stopped.");

	// Wait for m_Device to be finished
	vkDeviceWaitIdle(static_cast<VkDevice>(m_Device));

	/* Destroy renderer */
	g_Logger->debug("Destroying renderer...");

	for (size_t i = 0; i < bufferedFrameCount; i++)
	{
		vkDestroySemaphore(static_cast<VkDevice>(m_Device), static_cast<VkSemaphore>(imageAvailableSemaphores[i]), nullptr);
		vkDestroySemaphore(static_cast<VkDevice>(m_Device), static_cast<VkSemaphore>(renderFinishedSemaphores[i]), nullptr);
		vkDestroyFence(static_cast<VkDevice>(m_Device), static_cast<VkFence>(bufferedFences[i]), nullptr);
	}

	terminate(m_Renderer);
	delete m_Renderer;
}

void glacier::Application::stop()
{
	m_Window->close();
}

void glacier::Application::framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
	/* Window resize, called during resize */
	glacier::Application* _this = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));

	_this->m_Renderer->destroy();
	_this->m_Renderer->create();

	/* Render */

	// Wait until the next frame should be drawn
	vkWaitForFences(static_cast<VkDevice>(_this->m_Device), 1, reinterpret_cast<VkFence*>(&(_this->bufferedFences[_this->m_CurrentFrame])), VK_TRUE, UINT64_MAX);

	// Acquire a swapchain image
	uint32_t imageIndex;
	VkResult result = vkAcquireNextImageKHR(static_cast<VkDevice>(_this->m_Device), static_cast<VkSwapchainKHR>(_this->m_Renderer->m_Swapchain), UINT64_MAX, static_cast<VkSemaphore>(_this->imageAvailableSemaphores[_this->m_CurrentFrame]), VK_NULL_HANDLE, &imageIndex);

	// Check if the swapchain is valid
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error(fmt::format("Failed to acquire swapchain image (Returned {})", result));
	}

	// Wait until this frame can be rendered
	if (_this->bufferedImageFences[imageIndex] != VK_NULL_HANDLE)
	{
		vkWaitForFences(static_cast<VkDevice>(_this->m_Device), 1, reinterpret_cast<VkFence*>(&(_this->bufferedImageFences[imageIndex])), VK_TRUE, UINT64_MAX);
	}

	_this->bufferedImageFences[imageIndex] = _this->bufferedFences[_this->m_CurrentFrame];

	// Render the frame
	_this->render(_this->m_Renderer);

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore waitSemaphores[] = { static_cast<VkSemaphore>(_this->imageAvailableSemaphores[_this->m_CurrentFrame]) };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;

	// Renderer needs to have a pipeline bound
	if (_this->m_Renderer->m_CommandBuffers.empty())
	{
		g_Logger->error("Renderer has no pipeline bound");
		return;
	}

	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = reinterpret_cast<VkCommandBuffer*>(&_this->m_Renderer->m_CommandBuffers[imageIndex]); //&commandBuffers[imageIndex];

	VkSemaphore signalSemaphores[] = { static_cast<VkSemaphore>(_this->renderFinishedSemaphores[_this->m_CurrentFrame]) };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	vkResetFences(static_cast<VkDevice>(_this->m_Device), 1, reinterpret_cast<VkFence*>(&(_this->bufferedFences[_this->m_CurrentFrame])));
	result = vkQueueSubmit(static_cast<VkQueue>(_this->m_Renderer->m_GraphicsQueue), 1, &submitInfo, static_cast<VkFence>(_this->bufferedFences[_this->m_CurrentFrame]));
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error(fmt::format("Failed to submit draw command buffer (Returned {})", result));
	}

	// Present the frame
	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	VkSwapchainKHR swapchains[] = { static_cast<VkSwapchainKHR>(_this->m_Renderer->m_Swapchain) };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapchains; // TODO: &swapchain ?
	presentInfo.pImageIndices = &imageIndex;

	result = vkQueuePresentKHR(static_cast<VkQueue>(_this->m_Renderer->m_PresentationQueue), &presentInfo);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error(fmt::format("Failed to present queue (Returned {})", result));
	}

	_this->m_CurrentFrame = (_this->m_CurrentFrame + 1) % _this->m_Renderer->m_Images.size();
}

#pragma warning(pop)

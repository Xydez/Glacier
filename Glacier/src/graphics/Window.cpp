#include "graphics/Window.hpp"

#include <stdexcept>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

std::unordered_map<void*, glacier::Window*> glacier::Window::s_Windows;

glacier::Window::Window(const WindowCreateInfo& info)
	: m_IsFullscreen(false), m_LastPosX(0), m_LastPosY(0), m_LastWidth(0), m_LastHeight(0)
{
	if (!glfwInit())
		throw std::runtime_error("GLFW failed to initialize");

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, info.resizable ? GLFW_TRUE : GLFW_FALSE);

	m_Handle = glfwCreateWindow(info.width, info.height, info.title, nullptr, nullptr);
	if (m_Handle == nullptr)
		throw std::runtime_error("Failed to create a window");

	s_Windows.insert(std::make_pair(m_Handle, this));

	glfwSetWindowSizeLimits(static_cast<GLFWwindow*>(m_Handle),
		info.minWidth == 0 ? GLFW_DONT_CARE : info.minWidth,
		info.minHeight == 0 ? GLFW_DONT_CARE : info.minHeight,
		info.maxWidth == 0 ? GLFW_DONT_CARE : info.maxWidth,
		info.maxHeight == 0 ? GLFW_DONT_CARE : info.maxHeight
	);

	glfwSetKeyCallback(static_cast<GLFWwindow*>(m_Handle), [](GLFWwindow* window, int key, int scancode, int action, int mods)
		{
			glacier::Window* _this = s_Windows[window];

			if (key == GLFW_KEY_F11 && action == GLFW_RELEASE)
			{
				if (_this->m_IsFullscreen)
				{
					_this->m_IsFullscreen = false;

					GLFWmonitor* monitor = glfwGetWindowMonitor(window);
					const GLFWvidmode* vidMode = glfwGetVideoMode(monitor);

					glfwSetWindowMonitor(window, nullptr, _this->m_LastPosX, _this->m_LastPosY, _this->m_LastWidth, _this->m_LastHeight, vidMode->refreshRate);
				}
				else
				{
					_this->m_IsFullscreen = true;

					GLFWmonitor* monitor = glfwGetWindowMonitor(window);
					if (monitor == NULL)
					{
						monitor = glfwGetPrimaryMonitor();
					}

					const GLFWvidmode* vidMode = glfwGetVideoMode(monitor);

					glfwGetWindowSize(window, &(_this->m_LastWidth), &(_this->m_LastHeight));
					glfwGetWindowPos(window, &(_this->m_LastPosX), &(_this->m_LastPosY));

					glfwSetWindowMonitor(window, monitor, 0, 0, vidMode->width, vidMode->height, vidMode->refreshRate);
				}
			}
		});
}

glacier::Window::~Window()
{
	s_Windows.erase(m_Handle);

	glfwDestroyWindow(static_cast<GLFWwindow*>(m_Handle));
	glfwTerminate();
}

bool glacier::Window::isOpen() const
{
	return !glfwWindowShouldClose(static_cast<GLFWwindow*>(m_Handle));
}

void glacier::Window::close()
{
	glfwSetWindowShouldClose(static_cast<GLFWwindow*>(m_Handle), GLFW_TRUE);
}

glm::uvec2 glacier::Window::getSize() const
{
	int width;
	int height;

	glfwGetWindowSize(static_cast<GLFWwindow*>(m_Handle), &width, &height);

	if (width == 0 || height == 0)
		throw std::runtime_error("GLFW returned an invalid window size");

	return glm::uvec2(static_cast<unsigned int>(width), static_cast<unsigned int>(height));
}

glm::uvec2 glacier::Window::getFramebufferSize() const
{
	int width;
	int height;

	glfwGetFramebufferSize(static_cast<GLFWwindow*>(m_Handle), &width, &height);

	if (width < 0 || height < 0)
		throw std::runtime_error("GLFW returned an invalid framebuffer size");

	return glm::uvec2(static_cast<unsigned int>(width), static_cast<unsigned int>(height));
}

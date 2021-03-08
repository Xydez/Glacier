#include "graphics/Window.hpp"

#include <stdexcept>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

glacier::Window::Window(const WindowCreateInfo& info)
{
	if (!glfwInit())
		throw std::runtime_error("GLFW failed to initialize");

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, info.resizable ? GLFW_TRUE : GLFW_FALSE);

	m_Handle = glfwCreateWindow(info.width, info.height, info.title, nullptr, nullptr);
	if (m_Handle == nullptr)
		throw std::runtime_error("Failed to create a window");

	glfwSetWindowSizeLimits(static_cast<GLFWwindow*>(m_Handle),
		info.minWidth == 0 ? GLFW_DONT_CARE : info.minWidth,
		info.minHeight == 0 ? GLFW_DONT_CARE : info.minHeight,
		info.maxWidth == 0 ? GLFW_DONT_CARE : info.maxWidth,
		info.maxHeight == 0 ? GLFW_DONT_CARE : info.maxHeight
	);
}

glacier::Window::~Window()
{
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

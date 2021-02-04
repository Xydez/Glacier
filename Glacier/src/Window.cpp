#include "Window.hpp"

#include <stdexcept>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

glacier::Window::Window(const WindowInfo& info)
{
	if (!glfwInit())
		throw std::runtime_error("GLFW failed to initialize");

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, info.resizable ? GLFW_TRUE : GLFW_FALSE);

	m_Window = glfwCreateWindow(info.width, info.height, info.title, nullptr, nullptr);
	if (m_Window == nullptr)
		throw std::runtime_error("Failed to create a window");
}

glacier::Window::~Window()
{
	glfwDestroyWindow(static_cast<GLFWwindow*>(m_Window));
	glfwTerminate();
}

bool glacier::Window::isOpen() const
{
	return !glfwWindowShouldClose(static_cast<GLFWwindow*>(m_Window));
}

glm::uvec2 glacier::Window::getSize() const
{
	int width;
	int height;

	glfwGetWindowSize(static_cast<GLFWwindow*>(m_Window), &width, &height);

	if (width == 0 || height == 0)
		throw std::runtime_error("GLFW returned an invalid window size");

	return glm::uvec2(static_cast<unsigned int>(width), static_cast<unsigned int>(height));
}

glm::uvec2 glacier::Window::getFramebufferSize() const
{
	int width;
	int height;

	// Vulkan wants size in pixels, not screen coordinates, therefore we use glfwGetFramebufferSize instead of glfwGetWindowSize
	glfwGetFramebufferSize(static_cast<GLFWwindow*>(m_Window), &width, &height);

	if (width == 0 || height == 0)
		throw std::runtime_error("GLFW returned an invalid framebuffer size");

	return glm::uvec2(static_cast<unsigned int>(width), static_cast<unsigned int>(height));
}

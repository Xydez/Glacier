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

bool glacier::Window::isOpen()
{
	return !glfwWindowShouldClose(static_cast<GLFWwindow*>(m_Window));
}

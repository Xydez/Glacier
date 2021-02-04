#pragma once

#include <string>

#include <glm/vec2.hpp>

#include "common.hpp"

namespace glacier
{
	struct WindowInfo
	{
		const char* title;
		unsigned int width;
		unsigned int height;
		bool resizable;
	};

	class Window
	{
	public:
		GLACIER_API Window(const WindowInfo& info);
		GLACIER_API ~Window();

		// Delete copy
		Window(const Window&) = delete;
		Window& operator=(const Window&) = delete;

		// Delete move
		Window(Window&&) = delete;
		Window& operator=(Window&&) = delete;

		GLACIER_API bool isOpen() const;

		glm::uvec2 getSize() const;
		glm::uvec2 getFramebufferSize() const;

		friend class Application;
	private:
		void* m_Window;
	};
}
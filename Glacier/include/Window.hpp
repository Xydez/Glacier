#pragma once

#include <string>

#include "common.hpp"

namespace glacier
{
	struct WindowInfo
	{
		const char* title;
		unsigned int width;
		unsigned int height;
		bool resizable;
		bool vsync;
	};

	class Window
	{
	public:
		GLACIER_API Window(const WindowInfo& info);
		GLACIER_API ~Window();

		Window(const Window&) = delete;
		Window& operator=(const Window&) = delete;

		GLACIER_API bool isOpen();

		friend class Application;
	private:
		void* m_Window;
	};
}
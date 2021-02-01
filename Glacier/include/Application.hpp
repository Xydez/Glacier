#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <optional>

#include "Window.hpp"

namespace glacier
{
	struct ApplicationInfo
	{
		const char* name;

		unsigned int major;
		unsigned int minor;
		unsigned int patch;

		bool vsync;

		WindowInfo windowInfo;
	};

	class Application
	{
	public:
		GLACIER_API Application(const ApplicationInfo& info);
		GLACIER_API ~Application();

		GLACIER_API void run();

		static std::shared_ptr<spdlog::logger> s_Logger;
	private:
		Window* m_Window;

		void* m_VulkanInstance;
		void* m_DebugMessenger;
		void* m_Device;
		void* m_Surface;
		void* m_Swapchain;

		void* m_GraphicsQueue;
		void* m_PresentationQueue;

		std::vector<void*> m_ImageViews;
	};
}
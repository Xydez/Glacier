#pragma once

#include <optional>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include "Window.hpp"
#include "PipelineInfo.hpp"

namespace glacier
{
	struct ApplicationInfo
	{
		const char* name;

		unsigned int major;
		unsigned int minor;
		unsigned int patch;

		bool vsync;

		WindowCreateInfo windowInfo;
	};

	class Application
	{
	public:
		GLACIER_API Application(const ApplicationInfo& info);
		GLACIER_API ~Application();

		GLACIER_API void run();
		GLACIER_API void stop();

		virtual void initialize(PipelineInfo& pipeline) {};

		virtual void update(double deltaTime) {}
		virtual void render() {}

		friend class Shader;
	private:
		ApplicationInfo m_Info;
		Window* m_Window;

		void* m_VulkanInstance;
		void* m_DebugMessenger;
		void* m_PhysicalDevice;
		void* m_Device;
		void* m_Surface;
		//void* m_Swapchain;
		//void* m_PipelineLayout;
		//void* m_RenderPass;
		//void* m_GraphicsPipeline;

		//void* m_GraphicsQueue;
		//void* m_PresentationQueue;

		//std::vector<void*> m_ImageViews;
	};
}
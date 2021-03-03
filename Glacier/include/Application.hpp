#pragma once

#include <optional>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include "Window.hpp"

namespace glacier
{
	class Renderer;

	/**
	 * @brief Information about how the application should be initialized.
	*/
	struct ApplicationInfo
	{
		const char* name;

		unsigned int major;
		unsigned int minor;
		unsigned int patch;

		bool vsync;

		WindowCreateInfo windowInfo;
	};

	/**
	 * @brief Main application class. All glacier applications extend this class.
	*/
	class Application
	{
	protected:
		/**
		 * @brief Create a new application
		 * @param info Information about how to initialize the application
		*/
		GLACIER_API Application(const ApplicationInfo& info);

		/**
		 * @brief Destroy this application
		*/
		GLACIER_API ~Application();
	public:
		/**
		 * @brief Run the main loop of this application.
		*/
		GLACIER_API void run();

		/**
		 * @brief Finish rendering the current frame, then stop the game loop.
		*/
		GLACIER_API void stop();

		/**
		 * @brief Initialize the application. Called before starting the main loop.
		 * @note Guranteed to only be called once, before rendering has begun.
		 * @see terminate()
		*/
		virtual void initialize() {};

		/**
		 * @brief Initialize the renderer. Called when the renderer needs to be initialized.
		 * @note Can be called multiple times, for example when the framebuffer is resized.
		 * @param renderer The new renderer
		 * @see terminateRenderer()
		*/
		virtual void initializeRenderer(Renderer* renderer) {};

		/**
		 * @brief Update the application. Called during the update stage of the main loop.
		 * @param deltaTime The time in seconds since the last update
		 * @see update()
		*/
		virtual void update(double deltaTime) {}

		/**
		 * @brief Render the application onto the window. Called during the render stage of the main loop.
		 * @see render()
		*/
		virtual void render(Renderer* renderer) {}

		/**
		 * @brief Terminate the renderer. Called when the renderer needs to be terminated.
		 * @note Can be called multiple times, for example when the framebuffer is resized.
		 * @param renderer The renderer to be terminated.
		 * @see initializeRenderer()
		*/
		virtual void terminateRenderer(Renderer* renderer) {}

		/**
		 * @brief Terminate the application. Called when the main loop has finished.
		 * @note Guranteed to only be called once, after the rendering has finished.
		 * @see initialize()
		*/
		virtual void terminate() {}
	private:
		ApplicationInfo m_Info;
		Window* m_Window;
		Renderer* m_Renderer;

		void* m_VulkanInstance;
		void* m_DebugMessenger;
		void* m_PhysicalDevice;
		void* m_Device;
		void* m_Surface;

		void* m_UniformBufferLayout;

		bool m_FramebufferResized;
		unsigned int m_CurrentFrame = 0;

		friend class VertexBuffer;
		friend class IndexBuffer;
		friend class UniformBuffer;
		friend class UniformBufferLayout;
		friend class Renderer;
		friend class Pipeline;
		friend class Shader;
	};
}
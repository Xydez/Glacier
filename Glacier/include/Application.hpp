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
		*/
		virtual void initialize() {};

		/**
		 * @brief Initialize the renderer. Called when the renderer needs to be initialized.
		 * @param renderer The new renderer
		*/
		virtual void initializeRenderer(Renderer* renderer) {};

		/**
		 * @brief Update the application. Called during the update stage of the main loop.
		 * @param deltaTime The time since the last update
		*/
		virtual void update(double deltaTime) {}

		/**
		 * @brief Render the application onto the window. Called during the render stage of the main loop.
		*/
		virtual void render(Renderer* renderer) {}

		/**
		 * @brief Terminate the renderer. Called when the renderer needs to be terminated.
		 * @param renderer The renderer to be terminated.
		*/
		virtual void terminateRenderer(Renderer* renderer) {}

		/**
		 * @brief Terminate the application. Called when the main loop has finished.
		*/
		virtual void terminate() {}

		friend class Shader;
	private:
		ApplicationInfo m_Info;
		Window* m_Window;
		Renderer* m_Renderer;

		/* Guranteed to be assigned */
		void* m_VulkanInstance;
		void* m_DebugMessenger;
		void* m_PhysicalDevice;
		void* m_Device;
		void* m_Surface;

		bool m_FramebufferResized;

		friend class VertexBuffer;
		friend class IndexBuffer;
		friend class Renderer;
		friend class Pipeline;
	};
}
#pragma once

#include <string>

#include <glm/vec2.hpp>

#include "common.hpp"

namespace glacier
{
	/**
	 * Struct containing info about the window to be created
	 */
	struct WindowCreateInfo
	{
		const char* title;
		unsigned int width;
		unsigned int height;
		bool resizable;
	};

	class Window
	{
	public:
		GLACIER_API Window(const WindowCreateInfo& info);
		GLACIER_API ~Window();

		// Delete copy
		Window(const Window&) = delete;
		Window& operator=(const Window&) = delete;

		// Delete move
		Window(Window&&) = delete;
		Window& operator=(Window&&) = delete;

		/**
		 * Get if the window is open
		 * @return True if the window is open, otherwise false
		 */
		GLACIER_API bool isOpen() const;

		/**
		 * Close the window
		 */
		GLACIER_API void close();

		/**
		 * Get the size of the window in screen coordinates.
		 * @see glm::uvec2
		 * @return An unsigned vector containing the size of the window.
		 */
		glm::uvec2 getSize() const;

		/**
		 * Ge the size of the window in pixels
		 * @see glm::uvec2
		 * @return An unsigned vector containing the size of the window.
		 */
		glm::uvec2 getFramebufferSize() const;

		friend class Application;
	private:
		void* m_Window;
	};
}
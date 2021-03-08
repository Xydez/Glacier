#pragma once

#include "core/common.hpp"
#include "BufferElement.hpp"
#include "LifecycleObject.hpp"

#include <vector>

namespace glacier
{
	class Application;

	/**
	 * @brief %Buffer containing the uniforms that will be passed to the shader on the GPU.
	 * @warning Must only be created in Application::initializeRenderer() and destroyed in Application::terminateRenderer()
	*/
	class UniformBuffer : public LifecycleObject
	{
	public:
		/**
		 * @brief Create a new uniform buffer.
		 * @warning Must only be called in Application::initializeRenderer()
		 * @param application The currently active application.
		 * @param size Size in bytes of the data that will be copied.
		*/
		GLACIER_API UniformBuffer(const Application* application, size_t size);

		/**
		 * @brief Destroy this uniform buffer.
		 * @warning Must only be called in Application::terminateRenderer()
		*/
		GLACIER_API ~UniformBuffer();

		GLACIER_API void create() override;
		GLACIER_API void destroy() override;

		/**
		 * @brief Set the data in the buffer.
		 * @note Slow, but guranteed to work if called once.
		 * @warning Must only be called in Application::initializeRenderer(), Application::update() or Application::render()
		 * @param data The data to be copied
		 * @see update()
		*/
		GLACIER_API void set(const void* data);

		/**
		 * @brief Update the data in the buffer.
		 * @note Fast, but must be called every frame or the data may be outdated.
		 * @warning Must only be called in Application::update() or Application::render()
		 * @param data The data to be copied
		 * @see set()
		*/
		GLACIER_API void update(const void* data);

		// Delete copy
		UniformBuffer(const UniformBuffer&) = delete;
		UniformBuffer& operator=(const UniformBuffer&) = delete;

		// Delete move
		UniformBuffer(UniformBuffer&&) = delete;
		UniformBuffer& operator=(UniformBuffer&&) = delete;
	private:
		const Application* m_Application;
		size_t m_Size;

		std::vector<void*> m_HandleVector;
		std::vector<void*> m_MemoryVector;
		std::vector<void*> m_DescriptorSets;

		friend class Pipeline;
		friend class Renderer;
	};
}

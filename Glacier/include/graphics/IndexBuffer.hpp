#pragma once

#include "core/common.hpp"
#include "graphics/LifecycleObject.hpp"

namespace glacier
{
	class Application;

	/**
	 * @brief %Buffer containing the indices of the vertices that should be rendered.
	 * @warning Must only be created in Application::initializeRenderer()
	*/
	class IndexBuffer : public LifecycleObject
	{
	public:
		/**
		 * @brief Create a new index buffer and copy data into it.
		 * @param application The currently running application.
		 * @param data Pointer to the indices to be copied
		 * @param count Count of indices to be copied
		 * @warning Must only be called in Application::initializeRenderer()
		*/
		GLACIER_API IndexBuffer(const Application* application, const uint32_t* data, size_t count);

		/**
		 * @brief Destroy this index buffer.
		 * @warning Must only be called in Application::terminateRenderer()
		*/
		GLACIER_API ~IndexBuffer();

		GLACIER_API void create() override;
		GLACIER_API void destroy() override;

		// Delete copy
		IndexBuffer(const IndexBuffer&) = delete;
		IndexBuffer& operator=(const IndexBuffer&) = delete;

		// Delete move
		IndexBuffer(IndexBuffer&&) = delete;
		IndexBuffer& operator=(IndexBuffer&&) = delete;

		inline size_t getCount() const
		{
			return m_Count;
		}
	private:
		const Application* m_Application;

		void* m_Handle;
		void* m_Memory;
		void* m_Data;

		size_t m_Count;

		friend class Application;
		friend class Renderer;
		friend class Pipeline;
	};
}

#pragma once

#include "common.hpp"
#include "BufferElement.hpp"

#include <vector>

struct VkVertexInputBindingDescription;
struct VkVertexInputAttributeDescription;

namespace glacier
{
	class Application;

	/**
	 * @brief Class describing the layout of a vertex buffer.
	*/
	class VertexBufferLayout
	{
	public:
		//VertexBufferLayout() {}
		//~VertexBufferLayout() {}

		/**
		 * @brief Push an element to the end of this layout.
		 * @param elementType Primitive type of the element to be pushed.
		 * @param count How many elements of type elementType to be pushed. Must be between 1 and 4.
		*/
		GLACIER_API void push(BufferElement elementType, uint32_t count);
	private:
		std::vector<std::pair<BufferElement, uint32_t>> m_Elements;

		VkVertexInputBindingDescription getBindingDescription() const;
		std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions() const;

		friend class Application;
		friend class Renderer;
		friend class Pipeline;
	};

	class VertexBuffer
	{
	public:
		GLACIER_API VertexBuffer(const Application* application, const void* data, uint64_t size, const VertexBufferLayout& layout);
		GLACIER_API ~VertexBuffer();

		// Delete copy
		VertexBuffer(const VertexBuffer&) = delete;
		VertexBuffer& operator=(const VertexBuffer&) = delete;

		// Delete move
		VertexBuffer(VertexBuffer&&) = delete;
		VertexBuffer& operator=(VertexBuffer&&) = delete;
	private:
		VertexBufferLayout m_Layout;

		const Application* m_Application;

		void* m_Handle;
		void* m_Memory;

		friend class Application;
		friend class Renderer;
		friend class Pipeline;
	};
}

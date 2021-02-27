#pragma once

#include "common.hpp"

#include <vector>

struct VkVertexInputBindingDescription;
struct VkVertexInputAttributeDescription;

namespace glacier
{
	class Application;

	enum class VertexBufferElement
	{
		Float, Int, UnsignedInt, Byte, UnsignedByte
	};

	class VertexBufferLayout
	{
	public:
		GLACIER_API VertexBufferLayout() {}
		GLACIER_API ~VertexBufferLayout() {}

		GLACIER_API void push(VertexBufferElement elementType, uint32_t count);
	private:
		std::vector<std::pair<VertexBufferElement, uint32_t>> m_Elements;

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
		VertexBuffer(VertexBuffer&& other) = delete;
		VertexBuffer& operator=(VertexBuffer&& other) = delete;
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

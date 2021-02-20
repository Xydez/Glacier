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
		std::vector<std::pair<VertexBufferElement, size_t>> m_Elements;

		VkVertexInputBindingDescription getBindingDescription() const;
		std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions() const;

		friend class Application;
	};

	class VertexBuffer
	{
	public:
		GLACIER_API VertexBuffer(const Application& application, const void* data, uint32_t size, const VertexBufferLayout& layout);
		GLACIER_API ~VertexBuffer();
	private:
		VertexBufferLayout m_Layout;

		const Application* m_Application;

		void* m_Handle;
		void* m_Memory;

		friend class Application;
	};
}

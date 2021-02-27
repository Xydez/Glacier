#pragma once

#include "common.hpp"

namespace glacier
{
	class Application;

	class IndexBuffer
	{
	public:
		GLACIER_API IndexBuffer(const Application* application, const uint32_t* data, uint64_t size);
		GLACIER_API ~IndexBuffer();

		// Delete copy
		IndexBuffer(const IndexBuffer&) = delete;
		IndexBuffer& operator=(const IndexBuffer&) = delete;

		// Delete move
		IndexBuffer(IndexBuffer&& other) = delete;
		IndexBuffer& operator=(IndexBuffer&& other) = delete;
	private:
		const Application* m_Application;

		void* m_Handle;
		void* m_Memory;

		friend class Application;
		friend class Renderer;
		friend class Pipeline;
	};
}

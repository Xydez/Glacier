#pragma once

#include "common.hpp"
#include "Shader.hpp"

#include <vector>
#include <unordered_map>

namespace glacier
{
	class Application;

	class Renderer
	{
	public:
		/**
		 * @brief Bind a graphics pipeline configuration to this renderer. There can only be one graphics pipeline bound at a time.
		 * @param pipeline The pipeline to be bound.
		 * @param count If the pipeline has an index buffer count is how many indices there are in the buffer. Otherwise count is how many vertices there are in the vertex buffer.
		*/
		GLACIER_API void bindPipeline(const Pipeline& pipeline, uint32_t count);

		/**
		 * @brief Unbind the currently bound graphics pipeline configuration.
		*/
		GLACIER_API void unbindPipeline();
	private:
		Application* m_Application;
		void* m_Swapchain;
		void* m_CommandPool;
		void* m_GraphicsQueue;
		void* m_PresentationQueue;
		void* m_RenderPass;
		std::vector<void*> m_CommandBuffers;
		std::vector<void*> m_Framebuffers;
		std::vector<void*> m_Images;
		std::vector<void*> m_ImageViews;
		std::vector<Pipeline*> m_Pipelines;

		Renderer(Application* application);
		~Renderer();

		// Delete copy
		inline Renderer(Renderer&) = delete;
		Renderer& operator=(Renderer&) = delete;

		// Delete move
		Renderer(Renderer&& other) = delete;
		Renderer& operator=(Renderer&& other) = delete;

		friend class Application;
		friend class Pipeline;
		friend class VertexBuffer;
		friend class IndexBuffer;
	};
}
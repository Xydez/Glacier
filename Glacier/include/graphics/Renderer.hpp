#pragma once

#include "core/common.hpp"
#include "Shader.hpp"
#include "LifecycleObject.hpp"

#include <vector>
#include <unordered_map>

namespace glacier
{
	class Application;
	class Pipeline;

	/**
	 * @brief Responsible for handling the state of the renderer.
	 * @note May be recreated in which case Application::terminateRenderer() and Application::initializeRenderer() will be called.
	*/
	class Renderer : public LifecycleObject
	{
	public:
		/**
		 * @brief Bind a graphics pipeline configuration to this renderer. There can only be one graphics pipeline bound at a time.
		 * @param pipeline The pipeline to be bound.
		 * @warning Must only be called in Application::initializeRenderer(), Application::update(), Application::render() or Application::terminateRenderer()
		*/
		GLACIER_API void bindPipeline(const Pipeline& pipeline);

		/**
		 * @brief Unbind the currently bound graphics pipeline configuration.
		 * @warning Must only be called in Application::initializeRenderer(), Application::update(), Application::render() or Application::terminateRenderer()
		*/
		GLACIER_API void unbindPipeline();
	private:
		Application* m_Application;
		void* m_Swapchain;
		void* m_CommandPool;
		void* m_GraphicsQueue;
		void* m_PresentationQueue;
		void* m_RenderPass;
		void* m_DescriptorPool;
		std::vector<void*> m_CommandBuffers;
		std::vector<void*> m_Framebuffers;
		std::vector<void*> m_Images;
		std::vector<void*> m_ImageViews;
		std::vector<Pipeline*> m_Pipelines;

		Renderer(Application* application);
		~Renderer();

		void create() override;
		void destroy() override;

		// Delete copy
		inline Renderer(Renderer&) = delete;
		Renderer& operator=(Renderer&) = delete;

		// Delete move
		Renderer(Renderer&&) = delete;
		Renderer& operator=(Renderer&&) = delete;

		friend class Application;
		friend class Pipeline;
		friend class VertexBuffer;
		friend class IndexBuffer;
		friend class UniformBuffer;
	};
}
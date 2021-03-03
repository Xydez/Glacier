#pragma once

#include "common.hpp"
#include "Shader.hpp"
#include "VertexBuffer.hpp"
#include "IndexBuffer.hpp"
#include "UniformBuffer.hpp"

#include <unordered_map>
#include <optional>

namespace glacier
{
	class Application;
	class Renderer;

	/**
	 * @brief A graphics pipeline configuration.
	 * @note There needs to be a separate Pipeline instance for every possible pipeline configuration.
	 * @warning Must only be created in Application::initializeRenderer() and destroyed in Application::terminateRenderer()
	*/
	class Pipeline
	{
	public:
		/**
		 * @brief Create a new graphics pipeline configuration
		 * @param application The main glacier application
		 * @param renderer The currently active renderer
		 * @param shaders A map of the basic shader types and a pointer to their respective shaders. At least one ShaderType::Vertex and ShaderType::Fragment must be bound.
		 * @param vertexBuffer A VertexBuffer corresponding to the current vertex input
		 * @warning Must only be called in Application::initializeRenderer()
		*/
		GLACIER_API Pipeline(const Application* application, const Renderer* renderer, const std::unordered_map<ShaderType, Shader*>& shaders, const std::optional<UniformBuffer*>& uniformBuffer, const VertexBuffer* vertexBuffer, const IndexBuffer* indexBuffer);

		/**
		 * @brief Destroy this graphics pipeline configuration
		 * @warning Must only be called in Application::terminateRenderer()
		*/
		GLACIER_API ~Pipeline();

		// Delete copy constructor and operator
		Pipeline(const Pipeline&) = delete;
		Pipeline& operator=(const Pipeline&) = delete;

		// Delete move constructor and operator
		Pipeline(Pipeline&&) = delete;
		Pipeline& operator=(Pipeline&&) = delete;
	private:
		void* m_PipelineLayout;
		void* m_Pipeline;

		const Application* m_Application;
		const VertexBuffer* m_VertexBuffer;
		const IndexBuffer* m_IndexBuffer;
		const std::optional<UniformBuffer*> m_UniformBuffer;
		const std::unordered_map<ShaderType, Shader*> m_Shaders;

		friend class Renderer;
	};
}

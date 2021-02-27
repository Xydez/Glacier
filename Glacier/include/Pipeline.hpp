#pragma once

#include "common.hpp"
#include "Shader.hpp"
#include "VertexBuffer.hpp"
#include "IndexBuffer.hpp"

#include <unordered_map>

namespace glacier
{
	class Application;
	class Renderer;

	/**
	 * @brief A graphics pipeline configuration. There needs to be a separate Pipeline instance for every pipeline configuration. Must only be created in initializeRenderer
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
		*/
		GLACIER_API Pipeline(const Application* application, const Renderer* renderer, const std::unordered_map<ShaderType, Shader*>& shaders, const VertexBuffer& vertexBuffer, const IndexBuffer& indexBuffer);

		/**
		 * @brief Destroy this graphics pipeline configuration
		*/
		GLACIER_API ~Pipeline();

		// Delete copy constructor and operator
		Pipeline(const Pipeline&) = delete;
		Pipeline& operator=(const Pipeline&) = delete;

		// Delete move constructor and operator
		Pipeline(Pipeline&& other) = delete;
		Pipeline& operator=(Pipeline&& other) = delete;
	private:
		void* m_PipelineLayout;
		void* m_Pipeline;

		const Application* m_Application;
		const VertexBuffer* m_VertexBuffer;
		const IndexBuffer* m_IndexBuffer;
		const std::unordered_map<ShaderType, Shader*> m_Shaders;

		friend class Renderer;
	};
}

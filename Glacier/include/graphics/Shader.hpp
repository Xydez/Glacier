#pragma once

#include "core/common.hpp"
#include "io/Buffer.hpp"

#include <string_view>

namespace glacier
{
	class Application;

	/**
	 * @brief Types of shaders
	*/
	enum class ShaderType
	{
		Vertex, Tesselation, Geometry, Fragment, Compute
	};

	/**
	 * @brief Class for handling a shader on the GPU.
	 * @note Can be added to a Pipeline.
	 * @warning Must only be created in Application::initializeRenderer() and destroyed in Application::terminateRenderer()
	*/
	class Shader
	{
	public:
		/**
		 * @brief Load file containing a shader and create it on the GPU
		 * @param application The currently active application.
		 * @param path Path to a compiled SPIR-V shader file.
		 * @warning Must only be called in Application::initializeRenderer()
		*/
		GLACIER_API Shader(const Application* application, std::string_view path);
		
		/**
		 * @brief Create a shader on the GPU.
		 * @param application The currently active application.
		 * @param buffer A buffer containing a compiled SPIR-V shader.
		 * @warning Must only be called in Application::initializeRenderer()
		*/
		GLACIER_API Shader(const Application* application, const Buffer& buffer);

		/**
		 * @brief Destroy this shader.
		 * @warning Must only be called in Application::terminateRenderer()
		*/
		GLACIER_API ~Shader();

		// Delete copy
		Shader(const Shader&) = delete;
		Shader& operator=(const Shader&) = delete;

		// Delete move
		Shader(Shader&&) = delete;
		Shader& operator=(Shader&&) = delete;
	private:
		const Application* m_Application;
		void* m_ShaderModule;

		friend class Application;
		friend class Renderer;
		friend class Pipeline;
	};
}

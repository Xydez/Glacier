#pragma once

#include "common.hpp"
#include "Buffer.hpp"

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

	class Shader
	{
	public:
		GLACIER_API Shader(const Application* application, std::string_view path);
		GLACIER_API Shader(const Application* application, const Buffer& buffer);
		GLACIER_API ~Shader();

		// Delete copy
		Shader(const Shader&) = delete;
		Shader& operator=(const Shader&) = delete;

		// Delete move
		Shader(Shader&& other) = delete;
		Shader& operator=(Shader&& other) = delete;
	private:
		const Application* m_Application;
		void* m_ShaderModule;

		friend class Application;
		friend class Renderer;
		friend class Pipeline;
	};
}
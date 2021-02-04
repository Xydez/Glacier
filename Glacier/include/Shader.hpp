#pragma once

#include "common.hpp"

#include <string_view>

namespace glacier
{
	class Application;

	enum class ShaderType
	{
		Vertex, Fragment
	};

	class Shader
	{
	public:
		GLACIER_API Shader(const Application& application, std::string_view path, const ShaderType& type);
		GLACIER_API ~Shader();

		// Delete copy
		Shader(const Shader&) = delete;
		Shader& operator=(const Shader&) = delete;

		// Delete move
		Shader(Shader&&) = delete;
		Shader& operator=(Shader&&) = delete;
	private:
		const Application& m_Application;
		void* m_ShaderModule;
		ShaderType m_Type;

		friend class Application;
	};
}
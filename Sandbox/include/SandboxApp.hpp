#pragma once

#include <glacier.hpp>

glacier::ApplicationInfo generateApplicationInfo()
{
	glacier::WindowCreateInfo windowInfo;
	windowInfo.title = "SandboxApp";
	windowInfo.width = 800;
	windowInfo.height = 600;
	windowInfo.resizable = true;

	glacier::ApplicationInfo info = { "SandboxApp", 0, 1, 0, true, windowInfo };

	return info;
}

class SandboxApp : public glacier::Application
{
public:
	SandboxApp()
		: Application(generateApplicationInfo())
	{
		m_VertexShader = new glacier::Shader(*this, "../Sandbox/assets/shaders/vertex.spv", glacier::ShaderType::Vertex);
		m_FragmentShader = new glacier::Shader(*this, "../Sandbox/assets/shaders/fragment.spv", glacier::ShaderType::Fragment);
	}

	~SandboxApp()
	{
		delete m_VertexShader;
		delete m_FragmentShader;
	}

	void initialize(glacier::Pipeline& pipeline) override
	{
		pipeline.shaders.push_back(m_VertexShader);
		pipeline.shaders.push_back(m_FragmentShader);
	}
private:
	glacier::Shader* m_VertexShader;
	glacier::Shader* m_FragmentShader;
};

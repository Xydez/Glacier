#pragma once

#include <glacier.hpp>

glacier::ApplicationInfo generateApplicationInfo()
{
	glacier::WindowCreateInfo windowInfo = { 0 };
	windowInfo.title = "SandboxApp";
	windowInfo.minWidth = 640;
	windowInfo.minHeight = 480;
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
		m_VertexShader = new glacier::Shader(*this, "shaders/vertex.spv", glacier::ShaderType::Vertex);
		m_FragmentShader = new glacier::Shader(*this, "shaders/fragment.spv", glacier::ShaderType::Fragment);

		float buffer[]{
			-0.5f,  0.5f, 0.0f,  1.0f, 0.0f, 0.0f,
			 0.0f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,
			 0.5f,  0.5f, 0.0f,  0.0f, 0.0f, 1.0f
		};

		glacier::VertexBufferLayout layout;
		layout.push(glacier::VertexBufferElement::Float, 3);
		layout.push(glacier::VertexBufferElement::Float, 3);

		m_VertexBuffer = new glacier::VertexBuffer(*this, buffer, sizeof(buffer), layout);
	}

	~SandboxApp()
	{
		delete m_VertexShader;
		delete m_FragmentShader;

		delete m_VertexBuffer;
	}

	void initialize(glacier::PipelineInfo& pipeline) override
	{
		pipeline.shaders.push_back(m_VertexShader);
		pipeline.shaders.push_back(m_FragmentShader);

		pipeline.vertexBuffer = m_VertexBuffer;
		pipeline.vertexCount = 3;
	}
private:
	glacier::Shader* m_VertexShader;
	glacier::Shader* m_FragmentShader;
	glacier::VertexBuffer* m_VertexBuffer;
};

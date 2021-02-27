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
private:
	unsigned int frames = 0;
	double timer = 0.0;
public:
	SandboxApp()
		: Application(generateApplicationInfo()), m_Pipeline(nullptr), m_VertexShaderSource(nullptr), m_FragmentShaderSource(nullptr), m_VertexShader(nullptr), m_FragmentShader(nullptr), m_VertexBuffer(nullptr), m_IndexBuffer(nullptr)
	{}

	~SandboxApp()
	{}

	void initialize() override
	{
		m_VertexShaderSource = glacier::File("shaders/vertex.spv").read_ptr();
		m_FragmentShaderSource = glacier::File("shaders/fragment.spv").read_ptr();
	}

	void initializeRenderer(glacier::Renderer* renderer) override
	{
		float vertexBuffer[]{
			-0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 1.0f,
			 0.5f, -0.5f, 0.0f,  1.0f, 0.0f, 1.0f,
			 0.5f,  0.5f, 0.0f,  1.0f, 1.0f, 0.0f,
			-0.5f,  0.5f, 0.0f,  0.0f, 0.0f, 0.0f
		};

		unsigned int indexBuffer[]{
			0, 1, 2,
			0, 2, 3
		};

		glacier::VertexBufferLayout layout;
		layout.push(glacier::VertexBufferElement::Float, 3);
		layout.push(glacier::VertexBufferElement::Float, 3);

		m_VertexBuffer = new glacier::VertexBuffer(this, vertexBuffer, sizeof(vertexBuffer), layout);
		m_IndexBuffer = new glacier::IndexBuffer(this, indexBuffer, 6 * sizeof(unsigned int));

		m_VertexShader = new glacier::Shader(this, *m_VertexShaderSource);
		m_FragmentShader = new glacier::Shader(this, *m_FragmentShaderSource);

		std::unordered_map<glacier::ShaderType, glacier::Shader*> shaders;
		shaders.insert(std::make_pair(glacier::ShaderType::Vertex, m_VertexShader));
		shaders.insert(std::make_pair(glacier::ShaderType::Fragment, m_FragmentShader));

		m_Pipeline = new glacier::Pipeline(this, renderer, shaders, *m_VertexBuffer, *m_IndexBuffer);

		renderer->bindPipeline(*m_Pipeline, 6);
	}

	void update(double delta) override {}

	void render(glacier::Renderer* renderer) override {}

	void terminateRenderer(glacier::Renderer* renderer) override
	{
		renderer->unbindPipeline();

		delete m_VertexShader;
		delete m_FragmentShader;

		delete m_VertexBuffer;
		delete m_IndexBuffer;

		delete m_Pipeline;
	}

	void terminate() override
	{
		delete m_VertexShaderSource;
		delete m_FragmentShaderSource;
	}
private:
	glacier::Buffer* m_VertexShaderSource;
	glacier::Buffer* m_FragmentShaderSource;

	glacier::Shader* m_VertexShader;
	glacier::Shader* m_FragmentShader;

	glacier::VertexBuffer* m_VertexBuffer;
	glacier::IndexBuffer* m_IndexBuffer;

	glacier::Pipeline* m_Pipeline;
};

#pragma once

#include <glacier.hpp>

constexpr float PI = 3.1415926535;

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

struct TestUniform
{
	float u_ModifierA;
	float u_ModifierB;
	float u_ModifierC;
};

class SandboxApp : public glacier::Application
{
private:
	unsigned int frames = 0;
	double timer = 0.0;
public:
	SandboxApp()
		: Application(generateApplicationInfo()), m_Pipeline(nullptr), m_VertexShaderSource(nullptr), m_FragmentShaderSource(nullptr), m_VertexShader(nullptr), m_FragmentShader(nullptr), m_VertexBuffer(nullptr), m_IndexBuffer(nullptr), m_UniformBuffer(nullptr)
	{}

	~SandboxApp()
	{}

	void initialize(glacier::Renderer* renderer) override
	{
		m_VertexShaderSource = glacier::File("shaders/vertex.spv").read_ptr();
		m_FragmentShaderSource = glacier::File("shaders/fragment.spv").read_ptr();

		float vertexBuffer[]{
			-1.0f, -1.0f, 0.0f,  1.0f, 0.0f, 0.0f,
			 1.0f, -1.0f, 0.0f,  0.0f, 1.0f, 0.0f,
			 1.0f,  1.0f, 0.0f,  0.0f, 0.0f, 1.0f,
			-1.0f,  1.0f, 0.0f,  0.0f, 1.0f, 0.0f
		};

		unsigned int indexBuffer[]{
			0, 1, 2,
			0, 2, 3
		};

		glacier::VertexBufferLayout layout;
		layout.push(glacier::BufferElement::Float, 3);
		layout.push(glacier::BufferElement::Float, 3);

		m_VertexBuffer = new glacier::VertexBuffer(this, vertexBuffer, sizeof(vertexBuffer), layout);
		m_IndexBuffer = new glacier::IndexBuffer(this, indexBuffer, 6);

		m_VertexShader = new glacier::Shader(this, *m_VertexShaderSource);
		m_FragmentShader = new glacier::Shader(this, *m_FragmentShaderSource);

		std::unordered_map<glacier::ShaderType, glacier::Shader*> shaders;
		shaders.insert(std::make_pair(glacier::ShaderType::Vertex, m_VertexShader));
		shaders.insert(std::make_pair(glacier::ShaderType::Fragment, m_FragmentShader));

		m_UniformBuffer = new glacier::UniformBuffer(this, sizeof(TestUniform));

		m_Pipeline = new glacier::Pipeline(this, shaders, m_UniformBuffer, m_VertexBuffer, m_IndexBuffer);

		renderer->addPipeline(m_Pipeline);
	}

	void update(double delta) override
	{
		m_Timer += delta;

		TestUniform uniform = {
			static_cast<float>((cos(m_Timer) + 1.0) / 2.0),
			static_cast<float>(cos(m_Timer + 2.0 * PI / 3.0) / 2.0),
			static_cast<float>(cos(m_Timer + 4.0 * PI / 3.0) / 2.0)
		};
		m_UniformBuffer->update(&uniform);
	}

	void render(glacier::Renderer* renderer) override {}

	void terminate(glacier::Renderer* renderer) override
	{
		renderer->removePipeline(m_Pipeline);

		delete m_UniformBuffer;

		delete m_VertexShader;
		delete m_FragmentShader;

		delete m_VertexBuffer;
		delete m_IndexBuffer;

		delete m_Pipeline;

		delete m_VertexShaderSource;
		delete m_FragmentShaderSource;
	}
private:
	glacier::Buffer* m_VertexShaderSource;
	glacier::Buffer* m_FragmentShaderSource;

	glacier::Shader* m_VertexShader;
	glacier::Shader* m_FragmentShader;

	glacier::UniformBuffer* m_UniformBuffer;

	glacier::VertexBuffer* m_VertexBuffer;
	glacier::IndexBuffer* m_IndexBuffer;

	glacier::Pipeline* m_Pipeline;

	double m_Timer = 0.0;
};

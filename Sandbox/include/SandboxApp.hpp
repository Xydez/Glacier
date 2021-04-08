#pragma once

#include <glacier.hpp>

constexpr float PI = 3.1415926535f;
constexpr float MOVEMENT_SPEED = 2.0f;
constexpr float FAST_MOVEMENT_SPEED = 8.0f;
constexpr float MOUSE_SENSITIVITY = 0.004f;

struct MatrixUniform
{
	glm::mat4 value;
};

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

	glm::vec2 mousePos;
public:
	SandboxApp()
		: Application(generateApplicationInfo()), mousePos(getMousePos())
	{}

	~SandboxApp()
	{}

	void onKeyboardEvent(glacier::KeyboardKey key, glacier::KeyboardAction action, uint32_t mods) override
	{
		if (key == glacier::KeyboardKey::Escape && action == glacier::KeyboardAction::Release)
		{
			stop();
		}
	}

	void onMouseMoveEvent(glm::vec2 pos)
	{
		glm::vec2 deltaPos = pos - mousePos;
		mousePos = pos;

		m_Camera->rotate(MOUSE_SENSITIVITY * deltaPos.y, MOUSE_SENSITIVITY * deltaPos.x);
	}

	void initialize(glacier::Renderer* renderer) override
	{
		setCursorMode(glacier::CursorMode::Disabled);

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

		glm::uvec2 size = getWindow().getFramebufferSize();

		//m_Camera = new glacier::OrthographicCamera(glm::vec3(0.0f, 0.0f, 2.0f), 0.0f, glm::radians(-90.0f), size.x / size.y);
		m_Camera = new glacier::PerspectiveCamera(glm::vec3(0.0f, 0.0f, 2.0f), 0.0f, glm::radians(-90.0f), size.x / size.y, glm::radians(70.0f));

		m_UniformBuffer = new glacier::UniformBuffer(this, sizeof(MatrixUniform));

		m_Pipeline = new glacier::Pipeline(this, shaders, m_UniformBuffer, m_VertexBuffer, m_IndexBuffer);

		renderer->addPipeline(m_Pipeline);
	}

	void update(double delta) override
	{
		m_Timer += delta;

		//m_Camera->rotate(0.0f, glm::radians(45.0f) * delta);
		bool fast_move = isKeyPressed(glacier::KeyboardKey::LeftShift);

		float speed = (fast_move ? FAST_MOVEMENT_SPEED : MOVEMENT_SPEED);

		if (isKeyPressed(glacier::KeyboardKey::W))
			m_Camera->moveLocal(speed * static_cast<float>(delta) * glm::vec3(0.0f, 0.0f, 1.0f));

		if (isKeyPressed(glacier::KeyboardKey::A))
			m_Camera->moveLocal(speed * static_cast<float>(delta) * glm::vec3(-1.0f, 0.0f, 0.0f));

		if (isKeyPressed(glacier::KeyboardKey::S))
			m_Camera->moveLocal(speed * static_cast<float>(delta) * glm::vec3(0.0f, 0.0f, -1.0f));

		if (isKeyPressed(glacier::KeyboardKey::D))
			m_Camera->moveLocal(speed * static_cast<float>(delta) * glm::vec3(1.0f, 0.0f, 0.0f));


		MatrixUniform uniform { m_Camera->getProjMatrix() * m_Camera->getViewMatrix() };
		//uniform.value = glm::mat4(1.0f);

		m_UniformBuffer->update(&uniform);
	}

	void render(glacier::Renderer* renderer) override {}

	void terminate(glacier::Renderer* renderer) override
	{
		renderer->removePipeline(m_Pipeline);

		delete m_Camera;

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
	glacier::Buffer* m_VertexShaderSource = nullptr;
	glacier::Buffer* m_FragmentShaderSource = nullptr;

	glacier::Shader* m_VertexShader = nullptr;
	glacier::Shader* m_FragmentShader = nullptr;

	glacier::UniformBuffer* m_UniformBuffer = nullptr;

	glacier::VertexBuffer* m_VertexBuffer = nullptr;
	glacier::IndexBuffer* m_IndexBuffer = nullptr;

	glacier::Pipeline* m_Pipeline = nullptr;

	glacier::Camera* m_Camera = nullptr;

	double m_Timer = 0.0;
};

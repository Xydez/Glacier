#pragma once

#include <optional>

#include "graphics/Window.hpp"

struct GLFWwindow;

namespace glacier
{
	class Renderer;

	/**
	 * @brief Information about how the application should be initialized.
	*/
	struct ApplicationInfo
	{
		const char* name;

		unsigned int major;
		unsigned int minor;
		unsigned int patch;

		bool vsync;

		WindowCreateInfo windowInfo;
	};

	enum class KeyboardAction : uint8_t
	{
		Release = 0, Press = 1, Repeat = 2
	};

	enum class ModifierKey : uint8_t
	{
		Shift		= 1 << 0,
		Control		= 1 << 1,
		Alt			= 1 << 2,
		Super		= 1 << 3,
		CapsLock	= 1 << 4,
		NumLock		= 1 << 5
	};

    enum class KeyboardKey : uint16_t
    {
        Space = 32,
        Apostrophe = 39,
        Comma = 44,
        Minus = 45,
        Period = 46,
        Slash = 47,
        Num0 = 48,
        Num1 = 49,
        Num2 = 50,
        Num3 = 51,
        Num4 = 52,
        Num5 = 53,
        Num6 = 54,
        Num7 = 55,
        Num8 = 56,
        Num9 = 57,
        Semicolon = 59,
        Equal = 61,
        A = 65,
        B = 66,
        C = 67,
        D = 68,
        E = 69,
        F = 70,
        G = 71,
        H = 72,
        I = 73,
        J = 74,
        K = 75,
        L = 76,
        M = 77,
        N = 78,
        O = 79,
        P = 80,
        Q = 81,
        R = 82,
        S = 83,
        T = 84,
        U = 85,
        V = 86,
        W = 87,
        X = 88,
        Y = 89,
        Z = 90,
        Left_bracket = 91,
        Backslash = 92,
        Right_bracket = 93,
        Grave_accent = 96,
        World_1 = 161,
        World_2 = 162,
        Escape = 256,
        Enter = 257,
        Tab = 258,
        Backspace = 259,
        Insert = 260,
        Delete = 261,
        Right = 262,
        Left = 263,
        Down = 264,
        Up = 265,
        Page_up = 266,
        Page_down = 267,
        Home = 268,
        End = 269,
        Caps_lock = 280,
        Scroll_lock = 281,
        Num_lock = 282,
        Print_screen = 283,
        Pause = 284,
        F1 = 290,
        F2 = 291,
        F3 = 292,
        F4 = 293,
        F5 = 294,
        F6 = 295,
        F7 = 296,
        F8 = 297,
        F9 = 298,
        F10 = 299,
        F11 = 300,
        F12 = 301,
        F13 = 302,
        F14 = 303,
        F15 = 304,
        F16 = 305,
        F17 = 306,
        F18 = 307,
        F19 = 308,
        F20 = 309,
        F21 = 310,
        F22 = 311,
        F23 = 312,
        F24 = 313,
        F25 = 314,
        Kp0 = 320,
        Kp1 = 321,
        Kp2 = 322,
        Kp3 = 323,
        Kp4 = 324,
        Kp5 = 325,
        Kp6 = 326,
        Kp7 = 327,
        Kp8 = 328,
        Kp9 = 329,
        KpDecimal = 330,
        KpDivide = 331,
        KpMultiply = 332,
        KpSubtract = 333,
        KpAdd = 334,
        KpEnter = 335,
        KpEqual = 336,
        LeftShift = 340,
        LeftControl = 341,
        LeftAlt = 342,
        LeftSuper = 343,
        RightShift = 344,
        RightControl = 345,
        RightAlt = 346,
        RightSuper = 347,
        Menu = 348
    };

    enum class MouseAction : uint8_t
    {
        Release = 0, Press = 1
    };

    enum class MouseButton : uint32_t
    {
        Button1 = 0,
        Button2 = 1,
        Button3 = 2,
        Button4 = 3,
        Button5 = 4,
        Button6 = 5,
        Button7 = 6,
        Button8 = 7,
        Left = Button1,
        Right = Button2,
        Middle = Button3
    };

    enum class CursorMode : uint8_t
    {
        Normal = 0, Hidden = 1, Disabled = 2
    };

	/**
	 * @brief Main application class. All glacier applications extend this class.
	*/
	class Application
	{
	protected:
		/**
		 * @brief Create a new application
		 * @param info Information about how to initialize the application
		*/
		GLACIER_API Application(const ApplicationInfo& info);

		/**
		 * @brief Destroy this application
		*/
		GLACIER_API ~Application();

		inline Window& getWindow() const
		{
			return *m_Window;
		}

        GLACIER_API glm::vec2 getMousePos() const;

        GLACIER_API bool isKeyPressed(KeyboardKey key) const;

        GLACIER_API void setCursorMode(CursorMode cursorMode);

        virtual void onKeyboardEvent(KeyboardKey key, KeyboardAction action, uint32_t mods) {}
        virtual void onMouseButtonEvent(MouseButton button, MouseAction action, uint32_t mods) {}
        virtual void onMouseMoveEvent(glm::vec2 position) {}

        /**
         * @brief Initialize the application. Called before starting the main loop.
         * @note Guranteed to only be called once, before rendering has begun.
         * @see terminate()
        */
        virtual void initialize(Renderer* renderer) {};

        /**
         * @brief Update the application. Called during the update stage of the main loop.
         * @param deltaTime The time in seconds since the last update
         * @see update()
        */
        virtual void update(double deltaTime) {}

        /**
         * @brief Render the application onto the window. Called during the render stage of the main loop.
         * @see render()
        */
        virtual void render(Renderer* renderer) {}

        /**
         * @brief Terminate the application. Called when the main loop has finished.
         * @note Guranteed to only be called once, after the rendering has finished.
         * @see initialize()
        */
        virtual void terminate(Renderer* renderer) {}
	public:
		/**
		 * @brief Run the main loop of this application.
		*/
		GLACIER_API void run();

		/**
		 * @brief Finish rendering the current frame, then stop the game loop.
		*/
		GLACIER_API void stop();
	private:
		static void framebufferSizeCallback(GLFWwindow* window, int width, int height);
		static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
        static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
        static void cursorPosCallback(GLFWwindow* window, double x, double y);

		void engine_render();

		ApplicationInfo m_Info;
		Window* m_Window;
		Renderer* m_Renderer;

		void* m_VulkanInstance;
		void* m_DebugMessenger;
		void* m_PhysicalDevice;
		void* m_Device;
		void* m_Surface;

		void* m_UniformBufferLayout;

		unsigned int m_CurrentFrame = 0;
		bool m_SuboptimalFlag = false;
		double m_LastTime;

		std::vector<void*> imageAvailableSemaphores;
		std::vector<void*> renderFinishedSemaphores;
		std::vector<void*> bufferedFences;
		std::vector<void*> bufferedImageFences;

		friend class VertexBuffer;
		friend class IndexBuffer;
		friend class UniformBuffer;
		friend class UniformBufferLayout;
		friend class Renderer;
		friend class Pipeline;
		friend class Shader;
	};
}
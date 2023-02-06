#include "WindowsWindow.h"
#include <GLFW/glfw3.h>
#include "Core/Core.h"

namespace DT
{
	static bool s_GLFWInitialized = false;

	Window* Window::Create(const WindowSpecification& specification)
	{
		return new WindowsWindow(specification);
	}

	WindowsWindow::WindowsWindow(const WindowSpecification& specification)
		: m_Specification(specification)
	{
		if (!s_GLFWInitialized)
		{
			int result = glfwInit();
			ASSERT(result);
			s_GLFWInitialized = true;
		}

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		m_GLFWWindow = glfwCreateWindow(m_Specification.Width, m_Specification.Height, m_Specification.Title.c_str(), nullptr, nullptr);

		InstallGLFWCallbacks();
	}

	void WindowsWindow::SetEventCallBack(const EventCallbackFn& callback) 
	{ 
		m_WindowData.Callback = callback; 
	}

	void WindowsWindow::ProcessEvents()
	{
		glfwPollEvents();
	}

	void WindowsWindow::InstallGLFWCallbacks()
	{
		glfwSetWindowUserPointer(m_GLFWWindow, &m_WindowData);

		// window resize callback
		glfwSetWindowSizeCallback(m_GLFWWindow, [](GLFWwindow* window, int width, int height)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
			data.Width = width;
			data.Height = height;

			WindowResizeEvent e(width, height);
			data.Callback(e);
		});

		// window close callback
		glfwSetWindowCloseCallback(m_GLFWWindow, [](GLFWwindow* window)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
			WindowClosedEvent e{};
			data.Callback(e);
		});

		// keyboard callback
		glfwSetKeyCallback(m_GLFWWindow, [](GLFWwindow* window, int key, int scancode, int action, int modes)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			switch (action)
			{
				case GLFW_PRESS: 
				{
					KeyPressedEvent e(key, 0);
					data.Callback(e);
					break;
				}
				case GLFW_RELEASE:
				{
					KeyReleasedEvent e(key);
					data.Callback(e);
					break;
				}
				case GLFW_REPEAT:
				{
					KeyPressedEvent e(key, 1);
					data.Callback(e);
					break;
				}
			}
		});

		// keyboard callback
		glfwSetCharCallback(m_GLFWWindow, [](GLFWwindow* window, unsigned int key)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			KeyTypedEvent e(key);
			data.Callback(e);
		});

		// mouse click callback
		glfwSetMouseButtonCallback(m_GLFWWindow, [](GLFWwindow* window, int button, int action, int mods)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			switch (action)
			{
				case GLFW_PRESS: 
				{
					MouseButtonPressedEvent e(button);
					data.Callback(e);
					break;
				}
				case GLFW_RELEASE:
				{
					MouseButtonReleasedEvent e(button);
					data.Callback(e);
					break;
				}
			}
		});

		// mouse scroll callback
		glfwSetScrollCallback(m_GLFWWindow, [](GLFWwindow* window, double offsetX, double offsetY)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
			
			MouseScrolledEvent e((float)offsetX, (float)offsetY);
			data.Callback(e);
		});

		// mouse cursor move callback
		glfwSetCursorPosCallback(m_GLFWWindow, [](GLFWwindow* window, double posX, double posY)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
		
			MouseMovedEvent e((int32)posX, (int32)posY);
			data.Callback(e);
		});
	}

	bool WindowsWindow::KeyIsPressed(KeyCode key) const
	{
		return glfwGetKey(m_GLFWWindow, (int)key) == GLFW_PRESS;
	}

	bool WindowsWindow::MouseIsPressed(MouseCode button) const
	{
		return glfwGetMouseButton(m_GLFWWindow, (int)button);
	}

	int32 WindowsWindow::GetMouseX() const
	{
		double posX;
		glfwGetCursorPos(m_GLFWWindow, &posX, nullptr);
		return (int32)std::floor(posX);
	}

	int32 WindowsWindow::GetMouseY() const
	{
		double posY;
		glfwGetCursorPos(m_GLFWWindow, nullptr, &posY);
		return (int32)std::floor(posY);
	}
}
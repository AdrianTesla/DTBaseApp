#include "WindowsWindow.h"
#include <GLFW/glfw3.h>
#include "Core/Core.h"

namespace DT
{
	static bool s_GLFWInitialized = false;
	static uint32 s_ActiveWindowsCount = 0u;

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

			LOG_TRACE("GLFW Version {}", glfwGetVersionString());
		}

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		m_GLFWWindow = glfwCreateWindow(m_Specification.Width, m_Specification.Height, m_Specification.Title.c_str(), nullptr, nullptr);
		s_ActiveWindowsCount++;

		m_WindowData.Width = m_Specification.Width;
		m_WindowData.Height = m_Specification.Height;

		InstallGLFWCallbacks();

		EnumerateDisplayModes();
	}

	WindowsWindow::~WindowsWindow()
	{
		s_ActiveWindowsCount--;
		if (s_ActiveWindowsCount == 0u)
			glfwTerminate();
	}

	void WindowsWindow::SetEventCallBack(const EventCallbackFn& callback) 
	{ 
		m_WindowData.Callback = callback; 
	}

	void WindowsWindow::ProcessEvents()
	{
		glfwPollEvents();
	}

	int32 WindowsWindow::GetWidth() const
	{
		return m_WindowData.Width;
	}

	int32 WindowsWindow::GetHeight() const
	{
		return m_WindowData.Height;
	}

	void WindowsWindow::Maximize()
	{
		glfwMaximizeWindow(m_GLFWWindow);
	}

	void WindowsWindow::ToFullscreen()
	{
		glfwSetWindowMonitor(m_GLFWWindow, glfwGetPrimaryMonitor(), 0, 0, 1280, 720, GLFW_DONT_CARE);
	}

	void WindowsWindow::ToWindowed()
	{
		glfwSetWindowMonitor(m_GLFWWindow, nullptr, 0, 0, 1280, 720, GLFW_DONT_CARE);
	}

	void WindowsWindow::FixedAspectRatio(int32 numerator, int32 denominator)
	{
		glfwSetWindowAspectRatio(m_GLFWWindow, numerator, denominator);
	}

	int32 WindowsWindow::GetMouseX() const
	{
		double xpos;
		glfwGetCursorPos(m_GLFWWindow, &xpos, nullptr);
		return (int32)xpos;
	}
	
	int32 WindowsWindow::GetMouseY() const
	{
		double ypos;
		glfwGetCursorPos(m_GLFWWindow, nullptr, &ypos);
		return (int32)ypos;
	}

	void WindowsWindow::SetMousePosition(int32 x, int32 y)
	{
		glfwSetCursorPos(m_GLFWWindow, (double)x, (double)y);
	}

	std::string WindowsWindow::GetClipboardString() const
	{
		return glfwGetClipboardString(m_GLFWWindow);
	}

	void WindowsWindow::SetOpacity(float opacityValue)
	{
		glfwSetWindowOpacity(m_GLFWWindow, opacityValue);
	}

	void WindowsWindow::EnumerateDisplayModes()
	{
		int32 count;
		const GLFWvidmode* videoModes = glfwGetVideoModes(glfwGetPrimaryMonitor(), &count);
		for (int32 i = 0u; i < count; i++)
		{
			LOG_INFO("Video mode {}", i);
			LOG_TRACE("   width       = {}", videoModes[i].width);
			LOG_TRACE("   height      = {}", videoModes[i].height);
			LOG_TRACE("   refreshRate = {}", videoModes[i].refreshRate);
			LOG_TRACE("   redBits     = {}", videoModes[i].redBits);
			LOG_TRACE("   greenBits   = {}", videoModes[i].greenBits);
			LOG_TRACE("   blueBits    = {}", videoModes[i].blueBits);
		}
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

		// window focus callback
		glfwSetWindowFocusCallback(m_GLFWWindow, [](GLFWwindow* window, int focused)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
			WindowFocusEvent e((bool)focused);
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
}
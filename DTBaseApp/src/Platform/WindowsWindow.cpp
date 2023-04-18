#include "WindowsWindow.h"
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include "Core/Core.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

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
			LOG_TRACE("GLFW Version {}", glfwGetVersionString());

			s_GLFWInitialized = true;
		}

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		m_GLFWWindow = glfwCreateWindow(m_Specification.Width, m_Specification.Height, m_Specification.Title.c_str(), nullptr, nullptr);
		s_ActiveWindowsCount++;

		m_WindowData.Width = m_Specification.Width;
		m_WindowData.Height = m_Specification.Height;

		InstallGLFWCallbacks();

		if (m_Specification.StartMaximized)
			Maximize();
		if (m_Specification.StartCentered)
			CenterWindow();
		if (m_Specification.StartFullscreen)
			ToFullscreen();

		SetResizable(m_Specification.IsResizable);
		SetDecorated(m_Specification.IsDecorated);
		SetOpacity(m_Specification.StartOpacity);
		SetIcon(m_Specification.IconPath);
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
		int width;
		glfwGetWindowSize(m_GLFWWindow, &width, nullptr);
		return width;
	}

	int32 WindowsWindow::GetHeight() const
	{
		int height;
		glfwGetWindowSize(m_GLFWWindow, nullptr, &height);
		return height;
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

	void WindowsWindow::SetFixedAspectRatio(int32 numerator, int32 denominator)
	{
		glfwSetWindowAspectRatio(m_GLFWWindow, numerator, denominator);
	}

	int32 WindowsWindow::GetMouseX() const
	{
		double xpos;
		glfwGetCursorPos(m_GLFWWindow, &xpos, nullptr);
		return (int32)std::floor(xpos);
	}
	
	int32 WindowsWindow::GetMouseY() const
	{
		double ypos;
		glfwGetCursorPos(m_GLFWWindow, nullptr, &ypos);
		return (int32)std::floor(ypos);
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

	void WindowsWindow::SetTitle(const std::string& title)
	{
		glfwSetWindowTitle(m_GLFWWindow, title.c_str());
	}

	void WindowsWindow::SetDecorated(bool isDecorated)
	{
		glfwSetWindowAttrib(m_GLFWWindow, GLFW_DECORATED, isDecorated ? GLFW_TRUE : GLFW_FALSE);
	}

	void WindowsWindow::SetResizable(bool isResizable)
	{
		glfwSetWindowAttrib(m_GLFWWindow, GLFW_RESIZABLE, isResizable ? GLFW_TRUE : GLFW_FALSE);
	}

	void WindowsWindow::SetSize(int32 width, int32 height)
	{
		glfwSetWindowSize(m_GLFWWindow, width, height);
	}

	void WindowsWindow::SetPosition(int32 x, int32 y)
	{
		glfwSetWindowPos(m_GLFWWindow, x, y);
	}

	void WindowsWindow::CenterWindow()
	{
		const GLFWvidmode* videoMode = glfwGetVideoMode(glfwGetPrimaryMonitor());

		int windowWidth, windowHeight;
		glfwGetWindowSize(m_GLFWWindow, &windowWidth, &windowHeight);

		int32 x = (videoMode->width - windowWidth) / 2;
		int32 y = (videoMode->height - windowHeight) / 2;
		glfwSetWindowPos(m_GLFWWindow, x, y);
	}

	void WindowsWindow::SetSizeLimits(int32 minWidth, int32 minHeight, int32 maxWidth, int32 maxHeight)
	{
		glfwSetWindowSizeLimits(m_GLFWWindow, minWidth, minHeight, maxWidth, maxHeight);
	}

	void WindowsWindow::SetIcon(const std::filesystem::path& iconPath)
	{
		if (!std::filesystem::exists(iconPath))
			return;

		GLFWimage icon;
		int width, height, channels;
		icon.pixels = stbi_load(iconPath.string().c_str(), &width, &height, &channels, 4);
		icon.width = width;
		icon.height = height;
		glfwSetWindowIcon(m_GLFWWindow, 1, &icon);
	}

	Extent WindowsWindow::GetDisplayResolution() const
	{
		const GLFWvidmode* videoMode = glfwGetVideoMode(glfwGetPrimaryMonitor());
		return { videoMode->width,videoMode->height };
	}

	void* WindowsWindow::GetNativeWindow() const
	{
		return (void*)glfwGetWin32Window(m_GLFWWindow);
	}

	void WindowsWindow::EnumerateDisplayModes()
	{
		int32 modeCount;
		const GLFWvidmode* videoModes = glfwGetVideoModes(glfwGetPrimaryMonitor(), &modeCount);
		for (int32 i = 0u; i < modeCount; i++)
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
		
			MouseMovedEvent e((int32)std::floor(posX), (int32)std::floor(posY));
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
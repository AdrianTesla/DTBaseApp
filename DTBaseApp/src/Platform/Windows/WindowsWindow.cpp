#include "WindowsWindow.h"
#include <GLFW/glfw3.h>
#include "Core/Core.h"

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <stb_image.h>

namespace DT
{
	static bool s_GLFWInitialized = false;
	static uint32 s_ActiveWindowsCount = 0u;

	Ref<Window> Window::Create(const WindowSpecification& specification)
	{
		return Ref<WindowsWindow>::Create(specification);
	}

	WindowsWindow::WindowsWindow(const WindowSpecification& specification)
		: m_Specification(specification)
	{
		if (!s_GLFWInitialized)
		{
			int result = glfwInit();
			ASSERT(result);
			s_GLFWInitialized = true;

			LOG_TRACE("Initialized GLFW. Version {}", glfwGetVersionString());
		}

		CreateAndSpawnWindow();
		InstallGLFWCallbacks();
	}

	void WindowsWindow::CreateAndSpawnWindow()
	{
		GLFWmonitor* monitor = glfwGetPrimaryMonitor();
		const GLFWvidmode* videoMode = glfwGetVideoMode(monitor);

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RED_BITS, videoMode->redBits);
		glfwWindowHint(GLFW_GREEN_BITS, videoMode->greenBits);
		glfwWindowHint(GLFW_BLUE_BITS, videoMode->blueBits);
		glfwWindowHint(GLFW_REFRESH_RATE, videoMode->refreshRate);
		glfwWindowHint(GLFW_MAXIMIZED, (int)m_Specification.StartMaximized);
		glfwWindowHint(GLFW_RESIZABLE, (int)m_Specification.IsResizable);

		if (m_Specification.StartFullscreen)
			m_GLFWWindow = glfwCreateWindow(videoMode->width, videoMode->height, m_Specification.Title.c_str(), monitor, nullptr);
		else
			m_GLFWWindow = glfwCreateWindow(m_Specification.Width, m_Specification.Height, m_Specification.Title.c_str(), nullptr, nullptr);

		if (m_Specification.StartCentered)
			CenterWindow();

		if (!m_Specification.IsDecorated)
			SetDecorated(false);

		if (m_Specification.Opacity != 1.0f)
			SetOpacity(m_Specification.Opacity);

		SetIcon(m_Specification.IconPath);

		glfwGetWindowSize(m_GLFWWindow, &m_WindowData.Width, &m_WindowData.Height);

		s_ActiveWindowsCount++;
	}

	WindowsWindow::~WindowsWindow()
	{
		glfwDestroyWindow(m_GLFWWindow);
		s_ActiveWindowsCount--;

		if (s_ActiveWindowsCount == 0u)
			glfwTerminate();
	}

	void WindowsWindow::SetEventCallBack(const EventCallbackFn& callback) 
	{ 
		m_WindowData.Callback = callback; 
	}

	void* WindowsWindow::GetPlatformWindow() const
	{
		return m_GLFWWindow;
	}

	void* WindowsWindow::GetNativeWindow() const
	{
		HWND hWnd = glfwGetWin32Window(m_GLFWWindow);
		return (void*)hWnd;
	}

	void WindowsWindow::ProcessEvents()
	{
		glfwPollEvents();
	}

	int32 WindowsWindow::GetWidth() const
	{
		int32 width;
		glfwGetWindowSize(m_GLFWWindow, &width, nullptr);
		return width;
	}

	int32 WindowsWindow::GetHeight() const
	{
		int32 height;
		glfwGetWindowSize(m_GLFWWindow, nullptr, &height);
		return height;
	}

	void WindowsWindow::Maximize()
	{
		glfwMaximizeWindow(m_GLFWWindow);
	}

	void WindowsWindow::ToFullscreen()
	{
		m_WindowData.PreviousWidth = m_WindowData.Width;
		m_WindowData.PreviousHeight = m_WindowData.Height;

		GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
		const GLFWvidmode* videoMode = glfwGetVideoMode(primaryMonitor);
		glfwSetWindowMonitor(m_GLFWWindow, primaryMonitor, 0, 0, videoMode->width, videoMode->height, videoMode->refreshRate);
	}

	void WindowsWindow::ToWindowed()
	{
		glfwSetWindowMonitor(m_GLFWWindow, nullptr, 0, 0, m_WindowData.PreviousWidth, m_WindowData.PreviousHeight, GLFW_DONT_CARE);
		CenterWindow();
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

		int32 windowWidth, windowHeight;
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
		if (std::filesystem::exists(iconPath))
		{
			GLFWimage image;
			image.pixels = stbi_load(iconPath.string().c_str(), &image.width, &image.height, nullptr, 4);
			glfwSetWindowIcon(m_GLFWWindow, 1, &image);
			stbi_image_free(image.pixels);
		}
		else
		{
			LOG_WARN("Could not load window icon! {}", iconPath.string());
		}
	}

	void WindowsWindow::ShowMessageBox(const std::string& title, const std::string& text)
	{
		MessageBoxA(nullptr, text.c_str(), title.c_str(), MB_OK | MB_ICONASTERISK);
	}

	Extent WindowsWindow::GetDisplayResolution() const
	{
		const GLFWvidmode* videoMode = glfwGetVideoMode(glfwGetPrimaryMonitor());
		return { videoMode->width,videoMode->height };
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

		// window maximized callback
		glfwSetWindowMaximizeCallback(m_GLFWWindow, [](GLFWwindow* window, int maximized)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
			
			if (maximized == GLFW_TRUE) {
				WindowMaximized e;
				data.Callback(e);
			} else {
				WindowRestoredDown e;
				data.Callback(e);
			}
		});

		// window maximized callback
		glfwSetWindowIconifyCallback(m_GLFWWindow, [](GLFWwindow* window, int iconified)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
			
			WindowIconified e((bool)iconified);
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
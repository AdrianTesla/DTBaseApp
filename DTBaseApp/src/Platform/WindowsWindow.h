#pragma once
#include "Core/Window.h"

struct GLFWwindow;

namespace DT
{
	class WindowsWindow : public Window
	{
	public:
		WindowsWindow(const WindowSpecification& specification);

		virtual void SetEventCallBack(const EventCallbackFn& callback) override;
		virtual void ProcessEvents() override;

		bool KeyIsPressed(KeyCode key) const;
		bool MouseIsPressed(MouseCode button) const;

		int32 GetMouseX() const;
		int32 GetMouseY() const;
	private:
		void InstallGLFWCallbacks();
	private:
		struct WindowData
		{
			int32 Width;
			int32 Height;

			EventCallbackFn Callback;
		};
		WindowData m_WindowData;
		GLFWwindow* m_GLFWWindow;

		WindowSpecification m_Specification;
	};
}
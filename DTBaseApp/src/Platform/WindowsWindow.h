#pragma once
#include "Core/Window.h"

struct GLFWwindow;

namespace DT
{
	class WindowsWindow : public Window
	{
	public:
		WindowsWindow(const WindowSpecification& specification);
		virtual ~WindowsWindow() override;

		virtual void SetEventCallBack(const EventCallbackFn& callback) override;
		virtual void ProcessEvents() override;

		virtual int32 GetWidth() const override;
		virtual int32 GetHeight() const override;
		virtual void Maximize() override;
		virtual void ToFullscreen() override;
		virtual void ToWindowed() override;
		virtual void FixedAspectRatio(int32 numerator, int32 denominator) override;
		virtual int32 GetMouseX() const override;
		virtual int32 GetMouseY() const override;
		virtual void SetMousePosition(int32 x, int32 y) override;
		virtual std::string GetClipboardString() const override;
		virtual void SetOpacity(float opacityValue) override;

		void EnumerateDisplayModes();

		bool KeyIsPressed(KeyCode key) const;
		bool MouseIsPressed(MouseCode button) const;
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
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
		virtual const WindowSpecification& GetSpecification() const { return m_Specification; };

		virtual void ProcessEvents() override;
		virtual void Maximize() override;
		virtual void CenterWindow() override;
		virtual void ToFullscreen() override;
		virtual void ToWindowed() override;
		virtual int32 GetWidth() const override;
		virtual int32 GetHeight() const override;
		virtual int32 GetMouseX() const override;
		virtual int32 GetMouseY() const override;
		virtual std::string GetClipboardString() const override;
		virtual Extent GetDisplayResolution() const override;
		virtual void* GetNativeWindow() const override;

		virtual void SetFixedAspectRatio(int32 numerator, int32 denominator) override;
		virtual void SetMousePosition(int32 x, int32 y) override;
		virtual void SetOpacity(float opacityValue) override;
		virtual void SetTitle(const std::string& title) override;
		virtual void SetDecorated(bool isDecorated) override;
		virtual void SetResizable(bool isResizable) override;
		virtual void SetSize(int32 width, int32 height) override;
		virtual void SetPosition(int32 x, int32 y) override;
		virtual void SetSizeLimits(int32 minWidth, int32 minHeight, int32 maxWidth, int32 maxHeight) override;
		virtual void SetIcon(const std::filesystem::path& iconPath) override;
		

		bool KeyIsPressed(KeyCode key) const;
		bool MouseIsPressed(MouseCode button) const;
	private:
		void EnumerateDisplayModes();
		void InstallGLFWCallbacks();
	private:
		struct WindowData
		{
			int32 Width = 0;
			int32 Height = 0;

			EventCallbackFn Callback;
		};
		WindowData m_WindowData;
		GLFWwindow* m_GLFWWindow;

		WindowSpecification m_Specification;
	};
}
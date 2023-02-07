#pragma once
#include "Event.h"

namespace DT
{
	struct WindowSpecification
	{
		std::string Title    = "Dodge This Base Application";
		uint32 Width         = 1280u;
		uint32 Height        = 720u;
		bool StartMaximized  = false;
		bool StartCentered   = true;
		bool StartFullscreen = false;
		bool IsResizable     = true;
		bool IsDecorated     = true;
		float StartOpacity   = 1.0f;
		std::string IconPath = "ApplicationIcon.png";
	};

	/* platform independent desktop window */
	class Window
	{
	public:
		using EventCallbackFn = std::function<void(Event&)>;

		static Window* Create(const WindowSpecification& specification);
		virtual ~Window() = default;

		virtual void SetEventCallBack(const EventCallbackFn& callback) = 0;
		virtual const WindowSpecification& GetSpecification() const = 0;

		virtual void ProcessEvents() = 0;
		virtual void Maximize() = 0;
		virtual void CenterWindow() = 0;
		virtual void ToFullscreen() = 0;
		virtual void ToWindowed() = 0;
		virtual int32 GetWidth() const = 0;
		virtual int32 GetHeight() const = 0;
		virtual int32 GetMouseX() const = 0;
		virtual int32 GetMouseY() const = 0;
		virtual std::string GetClipboardString() const = 0;
		virtual Extent GetDisplayResolution() const = 0;

		virtual void SetFixedAspectRatio(int32 numerator, int32 denominator) = 0;
		virtual void SetMousePosition(int32 x, int32 y) = 0;
		virtual void SetOpacity(float opacityValue) = 0;
		virtual void SetTitle(const std::string& title) = 0;
		virtual void SetDecorated(bool isDecorated) = 0;
		virtual void SetResizable(bool isResizable) = 0;
		virtual void SetSize(int32 width, int32 height) = 0;
		virtual void SetPosition(int32 x, int32 y) = 0;
		virtual void SetSizeLimits(int32 minWidth, int32 minHeight, int32 maxWidth, int32 maxHeight) = 0;
		virtual void SetIcon(const std::filesystem::path& iconPath) = 0;
	};
}
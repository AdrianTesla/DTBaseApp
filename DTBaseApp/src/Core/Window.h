#pragma once
#include "Event.h"

namespace DT
{
	struct WindowSpecification
	{
		std::string Title = "Vulkan Learning";
		uint32 Width      = 1280u;
		uint32 Height     = 720u;
	};

	/* platform independent desktop window */
	class Window
	{
	public:
		using EventCallbackFn = std::function<void(Event&)>;

		static Window* Create(const WindowSpecification& specification);
		virtual ~Window() = default;

		virtual void SetEventCallBack(const EventCallbackFn& callback) = 0;
		virtual void ProcessEvents() = 0;

		virtual int32 GetWidth() const = 0;
		virtual int32 GetHeight() const = 0;
		virtual void Maximize() = 0;
		virtual void ToFullscreen() = 0;
		virtual void ToWindowed() = 0;
		virtual void FixedAspectRatio(int32 numerator, int32 denominator) = 0;
		virtual int32 GetMouseX() const = 0;
		virtual int32 GetMouseY() const = 0;
		virtual void SetMousePosition(int32 x, int32 y) = 0;
		virtual std::string GetClipboardString() const = 0;
		virtual void SetOpacity(float opacityValue) = 0;
	};
}
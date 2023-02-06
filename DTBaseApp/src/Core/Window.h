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

	class Window
	{
	public:
		using EventCallbackFn = std::function<void(Event&)>;

		static Window* Create(const WindowSpecification& specification);
		virtual ~Window() = default;

		virtual void SetEventCallBack(const EventCallbackFn& callback) = 0;
		virtual void ProcessEvents() = 0;
	};
}
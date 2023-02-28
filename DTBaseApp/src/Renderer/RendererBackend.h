#pragma once
#include "Core/Core.h"

namespace DT
{
	class RendererBackend
	{
	public:
		virtual void Init() = 0;
		virtual void Shutdown() = 0;

		virtual void BeginFrame() = 0;
		virtual void EndFrame() = 0;

		virtual void OnWindowResize() = 0;
		virtual void SetVerticalSync(bool enabled) = 0;

		virtual uint32 CurrentFrame() const = 0;
	};
}
#pragma once
#include <Core/Window.h>

namespace DT
{
	class RendererContext : public RefCounted
	{
	public:
		virtual ~RendererContext() = default;

		virtual void Init() = 0;
		virtual void Shutdown() = 0;

		static Ref<RendererContext> Create(const Ref<Window>& window);
	};
}
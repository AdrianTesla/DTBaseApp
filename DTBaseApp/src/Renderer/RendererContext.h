#pragma once
#include "Core/Ref.h"
#include <Core/Window.h>

namespace DT
{
	class RendererContext : public RefCounted
	{
	public:
		static Ref<RendererContext> Create(const Ref<Window>& window);
		virtual ~RendererContext() = default;

		virtual void Init() = 0;
		virtual void Present() = 0;
		virtual void DoFrameTest() = 0;
	};
}
#pragma once
#include "Core/Ref.h"

namespace DT
{
	class RendererContext : public RefCounted
	{
	public:
		static Ref<RendererContext> Create();
		virtual ~RendererContext() = default;

		virtual void Init() = 0;
		virtual void Present() = 0;
	};
}
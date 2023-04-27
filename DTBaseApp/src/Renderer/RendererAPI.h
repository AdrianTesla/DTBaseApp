#pragma once
#include "RenderPass.h"

namespace DT
{
	class RendererAPI
	{
	public:
		virtual void Init() = 0;
		virtual void Shutdown() = 0;

		virtual void BeginFrame() = 0;
		virtual void EndFrame() = 0;

		virtual void BeginRenderPass(Ref<RenderPass> renderPass) = 0;
		virtual void EndRenderPass() = 0;

		virtual void OnResize(uint32 width, uint32 height) = 0;

		virtual void Draw(uint32 vertexCount) = 0;
	};
}

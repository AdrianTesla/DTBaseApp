#pragma once
#include "RendererAPI.h"

namespace DT
{
	class Renderer
	{
	public:
		static void Init();
		static void Shutdown();

		static void BeginFrame();
		static void EndFrame();

		static void BeginRenderPass(Ref<RenderPass> renderPass);
		static void EndRenderPass(); 

		static void DrawTriangle();
	private:
		inline static RendererAPI* s_RendererAPI = nullptr;
	};
}
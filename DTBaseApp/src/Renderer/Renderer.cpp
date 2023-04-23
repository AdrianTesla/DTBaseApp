#include "Renderer.h"
#include "DX11Renderer.h"

namespace DT
{
	void Renderer::Init()
	{
		s_RendererAPI = new DX11Renderer();
		s_RendererAPI->Init();
	}

	void Renderer::Shutdown()
	{
		delete s_RendererAPI;
	}

	void Renderer::BeginFrame()
	{
		s_RendererAPI->BeginFrame();
	}

	void Renderer::EndFrame()
	{
		s_RendererAPI->EndFrame();
	}

	void Renderer::BeginRenderPass(Ref<RenderPass> renderPass)
	{
		s_RendererAPI->BeginRenderPass(renderPass);
	}

	void Renderer::EndRenderPass()
	{
		s_RendererAPI->EndRenderPass();
	}

	void Renderer::OnResize(uint32 width, uint32 height)
	{
		s_RendererAPI->OnResize(width, height);
	}

	void Renderer::DrawTriangle()
	{
		s_RendererAPI->DrawTriangle();
	}

}

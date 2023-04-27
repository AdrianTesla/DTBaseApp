#include "Renderer.h"
#include "DX11Renderer.h"
#include "Renderer2D.h"

namespace DT
{
	void Renderer::Init()
	{
		s_RendererAPI = new DX11Renderer();
		s_RendererAPI->Init();

		Renderer2D::Init();
	}

	void Renderer::Shutdown()
	{
		Renderer2D::Shutdown();
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

	void Renderer::Draw(uint32 vertexCount)
	{
		s_RendererAPI->Draw(vertexCount);
	}
}

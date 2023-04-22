#include "DX11Renderer.h"

namespace DT
{
	void DX11Renderer::Init()
	{
		m_Device = GraphicsContext::GetDevice();
		m_Context = GraphicsContext::GetContext();
		m_Swapchain = GraphicsContext::GetSwapchain();
	}

	void DX11Renderer::Shutdown()
	{
	}

	void DX11Renderer::BeginFrame()
	{
	}

	void DX11Renderer::EndFrame()
	{
		DXCALL(m_Swapchain->Present(1u, 0u));
	}

	void DX11Renderer::BeginRenderPass(Ref<RenderPass> renderPass)
	{
		renderPass->GetSpecification().TargetFrameBuffer->Bind();
		float clearColor[4];
		clearColor[0] = renderPass->GetSpecification().ClearColor.r; //R
		clearColor[1] = renderPass->GetSpecification().ClearColor.g; //G
		clearColor[2] = renderPass->GetSpecification().ClearColor.b; //B
		clearColor[3] = renderPass->GetSpecification().ClearColor.a; //A
		m_Context->ClearRenderTargetView(renderPass->GetSpecification().TargetFrameBuffer->GetRTV(), clearColor);
	}

	void DX11Renderer::EndRenderPass()
	{
	}

	void DX11Renderer::DrawTriangle()
	{
	}
}
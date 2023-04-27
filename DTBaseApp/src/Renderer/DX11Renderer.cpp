#include "DX11Renderer.h"
#include "Core/Application.h"
#include "DX11Buffers.h"
#include "Pipeline.h"

namespace DT
{
	struct DX11RendererData
	{
	};

	static DX11RendererData* s_RendererData = nullptr;

	void DX11Renderer::Init()
	{
		m_Device = GraphicsContext::GetDevice();
		m_Context = GraphicsContext::GetContext();
		m_Swapchain = GraphicsContext::GetSwapchain();

		s_RendererData = new DX11RendererData();
	}

	void DX11Renderer::Shutdown()
	{
		delete s_RendererData;
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
		//TODO! //Usa le dimensioni del framebuffer e non della finestra!
		D3D11_VIEWPORT viewport{};
		viewport.TopLeftX = 0.0f;
		viewport.TopLeftY = 0.0f;
		viewport.Width = (float)Application::Get().GetWindow().GetWidth();
		viewport.Height = (float)Application::Get().GetWindow().GetHeight();
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;

		m_Context->RSSetViewports(1u, &viewport);

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

	void DX11Renderer::OnResize(uint32 width, uint32 height)
	{
		GraphicsContext::OnResize(width, height);
	}

	void DX11Renderer::Draw(uint32 vertexCount)
	{
		m_Context->Draw(vertexCount, 0u);
	}
}
#include "DX11Renderer.h"
#include "Core/Application.h"
#include "DX11Buffers.h"
#include "Pipeline.h"

namespace DT
{
	struct DX11RendererData
	{
		Ref<VertexBuffer> TriangleVertexBuffer;
		Ref<Pipeline> TrianglePipeline;
	};

	static DX11RendererData* s_RendererData = nullptr;

	struct TriangleVertex
	{
		float x;
		float y;
	};

	void DX11Renderer::Init()
	{
		m_Device = GraphicsContext::GetDevice();
		m_Context = GraphicsContext::GetContext();
		m_Swapchain = GraphicsContext::GetSwapchain();

		s_RendererData = new DX11RendererData();

		TriangleVertex vertices[3];
		vertices[0] = { 0.0f,0.5f };
		vertices[1] = { 0.3f,-0.4f };
		vertices[2] = { -0.3f,-0.4f };

		//Upload vertices on GPU
		s_RendererData->TriangleVertexBuffer = CreateRef<VertexBuffer>(vertices, sizeof(vertices));
		s_RendererData->TrianglePipeline = CreateRef<Pipeline>(); 
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

	void DX11Renderer::DrawTriangle()
	{
		//Set Viewport
		D3D11_VIEWPORT viewport{};
		viewport.TopLeftX = 0.0f;
		viewport.TopLeftY = 0.0f;
		viewport.Width = (float)Application::Get().GetWindow().GetWidth();
		viewport.Height = (float)Application::Get().GetWindow().GetHeight();
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;

		m_Context->RSSetViewports(1u, &viewport);
		s_RendererData->TriangleVertexBuffer->Bind((uint32)sizeof(TriangleVertex));
		s_RendererData->TrianglePipeline->Bind();

		//Draw Triangle
		m_Context->Draw(3u, 0u);
	}
}
#include "DX11Renderer.h"
#include "Core/Application.h"
#include "DX11Buffers.h"
#include "Pipeline.h"

namespace DT
{
	struct DX11RendererData
	{
		Ref<VertexBuffer> QuadVertexBuffer;
	};

	static DX11RendererData* s_RendererData = nullptr;

	void DX11Renderer::Init()
	{
		m_Device = GraphicsContext::GetDevice();
		m_Context = GraphicsContext::GetContext();
		m_Swapchain = GraphicsContext::GetSwapchain();

		s_RendererData = new DX11RendererData();

		glm::vec2 vertices[6];
		vertices[0] = { -1.0f, 1.0f };
		vertices[1] = { 1.0f, 1.0f };
		vertices[2] = { 1.0f, -1.0f };

		vertices[3] = { -1.0f, 1.0f };
		vertices[4] = { 1.0f, -1.0f };
		vertices[5] = { -1.0f, -1.0f };
		s_RendererData->QuadVertexBuffer = CreateRef<VertexBuffer>(vertices, sizeof(vertices));
	}

	void DX11Renderer::Shutdown()
	{
		delete s_RendererData;
	}

	void DX11Renderer::BeginFrame()
	{
		if (m_ShouldResize)
		{
			GraphicsContext::OnResize(m_NewWidth, m_NewHeight);
			m_ShouldResize = false;
		}
	}

	void DX11Renderer::EndFrame()
	{
		HRESULT hr = m_Swapchain->Present(1u, 0u);

		if (hr != S_OK)
		{
			if (hr == DXGI_ERROR_DEVICE_REMOVED)
				OnResize(m_NewWidth, m_NewHeight);
		}
	}

	void DX11Renderer::BeginRenderPass(Ref<RenderPass> renderPass, bool explicitClear)
	{
		renderPass->Begin(explicitClear);
	}

	void DX11Renderer::EndRenderPass()
	{
	}

	void DX11Renderer::OnResize(int32 width, int32 height)
	{
		m_ShouldResize = true;
		m_NewWidth = width;
		m_NewHeight = height;
	}

	void DX11Renderer::Draw(uint32 vertexCount)
	{
		m_Context->Draw(vertexCount, 0u);
	}

	void DX11Renderer::DrawFullscreenQuad()
	{
		s_RendererData->QuadVertexBuffer->Bind(sizeof(glm::vec2));
		m_Context->Draw(6u, 0u);
	}
}
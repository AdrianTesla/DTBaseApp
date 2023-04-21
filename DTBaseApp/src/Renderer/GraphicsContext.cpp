#include "Core/Application.h"
#include "GraphicsContext.h"
#pragma comment (lib,"d3d11.lib")
#include "DX11Buffers.h"
#include "Pipeline.h"

namespace DT
{
	struct Vertex
	{
		float x;
		float y;
	};

	GraphicsContext::GraphicsContext(const Window* window)
	{
		m_Window = window;
		s_Instance = this;
	}

	void GraphicsContext::Init()
	{
		DXGI_SWAP_CHAIN_DESC swapchainDescriptor{};
		swapchainDescriptor.BufferCount = 2u;
		swapchainDescriptor.BufferDesc.Width = 0u;
		swapchainDescriptor.BufferDesc.Height = 0u;
		swapchainDescriptor.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		swapchainDescriptor.BufferDesc.RefreshRate.Denominator = 0u;
		swapchainDescriptor.BufferDesc.RefreshRate.Numerator = 0u;
		swapchainDescriptor.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		swapchainDescriptor.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		swapchainDescriptor.Windowed = TRUE;
		swapchainDescriptor.SampleDesc.Count = 1u;
		swapchainDescriptor.SampleDesc.Quality = 0u;
		swapchainDescriptor.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapchainDescriptor.Flags = 0u;
		swapchainDescriptor.OutputWindow = (HWND)m_Window->GetNativeWindow();

		UINT deviceCreateFlags = 0u;
#ifdef DT_DEBUG
		deviceCreateFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif // DT_DEBUG

		DXCALL(D3D11CreateDeviceAndSwapChain(
			nullptr,
			D3D_DRIVER_TYPE_HARDWARE,
			nullptr,
			deviceCreateFlags,
			nullptr,
			0u,
			D3D11_SDK_VERSION,
			&swapchainDescriptor,
			&m_Swapchain,
			&m_Device,
			nullptr,
			&m_Context
		));
		
		ID3D11Texture2D* pBackBuffer;
		DXCALL(m_Swapchain->GetBuffer(0u, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer));
		DXCALL(m_Device->CreateRenderTargetView(pBackBuffer, nullptr, &m_RenderTargetView))

		Vertex vertices[3];
		vertices[0] = { 0.0f,0.5f };
		vertices[1] = { 0.3f,-0.4f };
		vertices[2] = { -0.3f,-0.4f };

		//Upload vertices on GPU
		m_VertexBuffer = std::make_shared<VertexBuffer>(vertices, sizeof(vertices));
		m_Pipeline = std::make_shared<Pipeline>();
	}

	void GraphicsContext::Present()
	{
		DXCALL(m_Swapchain->Present(1u, 0u));
	}

	void GraphicsContext::BeginFrame()
	{
		float windowColor[4];
		windowColor[0] = 0.0f; //R
		windowColor[1] = 0.0f; //G
		windowColor[2] = 0.0f; //B
		windowColor[3] = 1.0f; //A
		m_Context->ClearRenderTargetView(m_RenderTargetView.Get(), windowColor);
	}

	void GraphicsContext::DrawTriangle()
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
		m_VertexBuffer->Bind(sizeof(Vertex));
		m_Pipeline->Bind();
		m_Context->OMSetRenderTargets(1u, m_RenderTargetView.GetAddressOf(), nullptr);

		//Draw Triangle
		m_Context->Draw(3u, 0u);
	}
}

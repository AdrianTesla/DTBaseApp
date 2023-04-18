#include "GraphicsContext.h"
#pragma comment (lib,"d3d11.lib")

namespace DT
{
	GraphicsContext::GraphicsContext(const Window* window)
	{
		m_Window = window;
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

		DXCALL(D3D11CreateDeviceAndSwapChain(
			nullptr,
			D3D_DRIVER_TYPE_HARDWARE,
			nullptr,
			0u,
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
		
		D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc{};
		renderTargetViewDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		renderTargetViewDesc.Texture2D.MipSlice = 0u;
		renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		DXCALL(m_Device->CreateRenderTargetView(pBackBuffer, nullptr, &m_RenderTargetView));
		int a = 0;
	}

	void GraphicsContext::Present()
	{
		DXCALL(m_Swapchain->Present(1u, 0u));
	}

	void GraphicsContext::BeginFrame()
	{
		float windowColor[4];
		windowColor[0] = 1.0f; //R
		windowColor[1] = 0.5f; //G
		windowColor[2] = 0.3f; //B
		windowColor[3] = 1.0f; //A
		m_Context->ClearRenderTargetView(m_RenderTargetView.Get(), windowColor);
	}
}

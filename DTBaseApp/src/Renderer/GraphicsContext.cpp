#include "Core/Application.h"
#include "GraphicsContext.h"
#include "Framebuffer.h"
#pragma comment (lib,"d3d11.lib")

namespace DT
{
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
		DXCALL(m_Device->CreateRenderTargetView(pBackBuffer, nullptr, &m_RenderTargetView));
		pBackBuffer->Release();
	}

	bool GraphicsContext::OnResize(int32 width, int32 height)
	{
		if (width < 1 || height < 1)
		{
			return false;
		}

		s_Instance->m_RenderTargetView->Release();
		DXCALL(s_Instance->m_Swapchain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0));
		
		ID3D11Texture2D* pBackBuffer;
		DXCALL(s_Instance->m_Swapchain->GetBuffer(0u, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer));
		DXCALL(s_Instance->m_Device->CreateRenderTargetView(pBackBuffer, nullptr, s_Instance->m_RenderTargetView.GetAddressOf()));
		pBackBuffer->Release();

		FramebufferPool::OnResize(width, height);
		return true;
	}
}

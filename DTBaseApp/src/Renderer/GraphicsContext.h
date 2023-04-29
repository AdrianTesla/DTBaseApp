#pragma once
#include "Core/Window.h"
#include <d3d11.h>
#include <wrl.h>

#define DXCALL(call) { HRESULT hr = (call); if (hr != S_OK) { LOG_ERROR("Doesn't work"); __debugbreak(); } }

namespace DT
{
	class GraphicsContext
	{
	public:
		GraphicsContext(const Window* window);
		void Init();

		static ID3D11Device* GetDevice() { return s_Instance->m_Device.Get(); }
		static ID3D11DeviceContext* GetContext() { return s_Instance->m_Context.Get(); }
		static ID3D11RenderTargetView* GetSwapchainRTV() { return s_Instance->m_RenderTargetView.Get(); }
		static IDXGISwapChain* GetSwapchain() { return s_Instance->m_Swapchain.Get(); };
		static bool OnResize(int32 width, int32 height);
	private:
		Microsoft::WRL::ComPtr<IDXGISwapChain> m_Swapchain;
		Microsoft::WRL::ComPtr<ID3D11Device> m_Device;
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_Context;
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_RenderTargetView;

		const Window* m_Window;
		inline static GraphicsContext* s_Instance = nullptr;
	};
}

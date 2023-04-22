#pragma once
#include "Core/Window.h"
#include <d3d11.h>
#include <wrl.h>

#define DXCALL(call) { HRESULT hr = (call); if (hr != S_OK) { LOG_ERROR("Doesn't work"); __debugbreak(); } }

namespace DT
{
	class Pipeline;
	class VertexBuffer;

	class GraphicsContext
	{
	public:
		GraphicsContext(const Window* window);
		void Init();
		void Present();
		void BeginFrame();
		void DrawTriangle();

		static ID3D11Device* GetDevice() { return s_Instance->m_Device.Get(); }
		static ID3D11DeviceContext* GetContext() { return s_Instance->m_Context.Get(); }
		static ID3D11RenderTargetView* GetSwapchainRTV() { return s_Instance->m_RenderTargetView.Get(); }
		static IDXGISwapChain* GetSwapchain() { return s_Instance->m_Swapchain.Get(); };
	private:
		Microsoft::WRL::ComPtr<IDXGISwapChain> m_Swapchain;
		Microsoft::WRL::ComPtr<ID3D11Device> m_Device;
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_Context;
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_RenderTargetView;
		Ref<VertexBuffer> m_VertexBuffer;
		Ref<Pipeline> m_Pipeline;

		const Window* m_Window;
		inline static GraphicsContext* s_Instance = nullptr;
	};
}

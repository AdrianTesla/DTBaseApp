#pragma once
#include "Core/Window.h"
#include <d3d11.h>
#include <wrl.h>

#define DXCALL(call) { HRESULT hr = (call); if (hr != S_OK) { LOG_ERROR("doesn't work"); __debugbreak(); } }

namespace DT
{
	class GraphicsContext
	{
	public:
		GraphicsContext(const Window* window);
		void Init();
		void Present();
		void BeginFrame();
		void DrawTriangle();

	private:
		Microsoft::WRL::ComPtr<IDXGISwapChain> m_Swapchain;
		Microsoft::WRL::ComPtr<ID3D11Device> m_Device;
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_Context;
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_RenderTargetView;
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_VertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11VertexShader> m_VertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> m_PixelShader;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> m_InputLayout;

		const Window* m_Window;
	};
}

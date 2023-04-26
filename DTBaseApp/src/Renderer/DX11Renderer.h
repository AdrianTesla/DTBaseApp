#pragma once
#include "RendererAPI.h"
#include "GraphicsContext.h"

namespace DT
{
	class DX11Renderer : public RendererAPI
	{
	public:
		virtual void Init() override;
		virtual void Shutdown() override;

		virtual void BeginFrame() override;
		virtual void EndFrame() override;

		virtual void BeginRenderPass(Ref<RenderPass> renderPass) override;
		virtual void EndRenderPass() override;

		virtual void OnResize(uint32 width, uint32 height) override;

		virtual void Draw(uint32 vertexCount) override;
		virtual void DrawTriangle() override;
	private:
		ID3D11DeviceContext* m_Context = nullptr;
		ID3D11Device* m_Device = nullptr;
		IDXGISwapChain* m_Swapchain = nullptr;	
	};
}

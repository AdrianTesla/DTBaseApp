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

		virtual void BeginRenderPass(Ref<RenderPass> renderPass, bool explicitClear) override;
		virtual void EndRenderPass() override;

		virtual void OnResize(int32 width, int32 height) override;

		virtual void Draw(uint32 vertexCount) override;
	private:
		ID3D11DeviceContext* m_Context = nullptr;
		ID3D11Device* m_Device = nullptr;
		IDXGISwapChain* m_Swapchain = nullptr;	

		bool m_ShouldResize = false;
		int32 m_NewWidth = 0;
		int32 m_NewHeight = 0;
	};
}

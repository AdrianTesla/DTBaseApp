#pragma once
#include "GraphicsContext.h"

namespace DT
{
	struct FramebufferSpecification
	{
		bool SwapchainTarget = true;
	};

	class Framebuffer
	{
	public:
		Framebuffer(const FramebufferSpecification& specification);
		void Bind();
		ID3D11RenderTargetView* GetRTV() const { return m_RenderTargetView; }
	private:
		ID3D11RenderTargetView* m_RenderTargetView = nullptr;
		FramebufferSpecification m_Specification;
	};
}

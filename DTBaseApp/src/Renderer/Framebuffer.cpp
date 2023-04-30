#include "Framebuffer.h"

namespace DT
{
	Framebuffer::Framebuffer(const FramebufferSpecification& specification)
		: m_Specification(specification)
	{
		FramebufferPool::AddFramebuffer(this);

		if (specification.SwapchainTarget)
		{
			m_RenderTargetView = GraphicsContext::GetSwapchainRTV();
		}
		else
		{
			//TODO!
		}
	}

	void Framebuffer::Bind()
	{
		GraphicsContext::GetContext()->OMSetRenderTargets(1u, &m_RenderTargetView, nullptr);
	}

	void Framebuffer::OnResize(int32 width, int32 height)
	{
		m_RenderTargetView = GraphicsContext::GetSwapchainRTV();
	}
}
#include "RenderingTestLayer.h"
#include "Renderer/Renderer.h"

namespace DT
{
	void RenderingTestLayer::OnAttach()
	{
		FramebufferSpecification speciFICAtion{};
		speciFICAtion.SwapchainTarget = true;
		m_Framebuffer = CreateRef<Framebuffer>(speciFICAtion);

		RenderPassSpecification renderPassSpecification{};
		renderPassSpecification.ClearColor = { 0.2f, 0.0f,0.4f,1.0f };
		renderPassSpecification.TargetFrameBuffer = m_Framebuffer;
		m_RenderPass = CreateRef<RenderPass>(renderPassSpecification);
	}

	void RenderingTestLayer::OnUpdate(float dt)
	{
	}

	void RenderingTestLayer::OnRender()
	{
		Renderer::BeginRenderPass(m_RenderPass);
		Renderer::DrawTriangle();
		Renderer::EndRenderPass();
	}

	void RenderingTestLayer::OnEvent(Event& event)
	{
	}

	void RenderingTestLayer::OnDetach()
	{
	}
}
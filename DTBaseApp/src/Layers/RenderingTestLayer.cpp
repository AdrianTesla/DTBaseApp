#include "RenderingTestLayer.h"
#include "Renderer/Renderer.h"
#include "Core/Application.h"
#include "Renderer/Renderer2D.h"

namespace DT
{
	struct Vertex
	{
		float x;
		float y;
	};

	void RenderingTestLayer::OnAttach()
	{
		Application::Get().GetWindow().SetSizeLimits(100,100,16'000,9'000);

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
		m_Time = m_Time + dt;

	}

	void RenderingTestLayer::OnRender()
	{

		Renderer::BeginRenderPass(m_RenderPass);

		Renderer2D::BeginScene();
		
		float y = 0.0f;

		for (uint32 i = 0u; i < 10u; i++)
		{
			Renderer2D::DrawQuad({ 0.0f, y }, 0.5f, 0.03f, {0.2f, 0.8f, y , 1.0f});
			y = y + 0.1f;
		}

		Renderer2D::EndScene();

		Renderer::EndRenderPass();
	}

	void RenderingTestLayer::OnEvent(Event& event)
	{
	}

	void RenderingTestLayer::OnDetach()
	{
	}
}
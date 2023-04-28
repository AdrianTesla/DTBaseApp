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
		renderPassSpecification.ClearColor = { Animate(1.0f), 0.1f, 0.4f, 1.0f};
		renderPassSpecification.TargetFrameBuffer = m_Framebuffer;
		m_RenderPass = CreateRef<RenderPass>(renderPassSpecification);
	}

	void RenderingTestLayer::OnUpdate(float dt)
	{
		m_Time = m_Time + dt;
		m_RenderPass->GetSpecification().ClearColor = { 0.5f * Animate(2.0f), 0.1f, 0.5f * Animate(1.0f), 1.0f};
	}

	void RenderingTestLayer::OnRender()
	{
		Renderer::BeginRenderPass(m_RenderPass);

		Renderer2D::BeginScene();

		Renderer2D::DrawQuad({ 0.5f, 0.5f }, 0.2f, 0.2f, { 0.0f, 0.4f, 0.9f, 1.0f });
		Renderer2D::DrawQuad({ -0.5f, 0.5f }, 0.2f, 0.5f * Animate(0.2f), {0.0f, 0.4f, 0.9f, 1.0f});
		Renderer2D::DrawQuad({ 0.0f, -0.5f }, 1.0f, 0.2f, { 0.0f, 0.4f, 0.9f, 1.0f });

		Renderer2D::DrawCircle({ 0.0f,0.0f }, Animate(0.5f), 0.1f, 0.05f, {Animate(1.0f), 0.8f, 0.9f, 0.5f});

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
#include "RenderingTestLayer.h"
#include "Renderer/Renderer.h"
#include "Core/Application.h"
#include "Renderer/Renderer2D.h"
#include "imgui.h"

namespace DT
{
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
		m_RenderPass->GetSpecification().ClearColor = { 0.15f * Animate(0.5f), 0.0f, 0.15f * Animate(0.3f), 1.0f};
	}

	void RenderingTestLayer::OnRender()
	{
		Renderer::BeginRenderPass(m_RenderPass);

		Renderer2D::BeginScene();

		//Renderer2D::DrawQuad({ 0.5f, 0.5f }, 0.2f, 0.2f, { 0.0f, 0.4f, 0.9f, 1.0f });
		//Renderer2D::DrawQuad({ -0.5f, 0.5f }, 0.2f, 0.5f * Animate(0.2f), {0.0f, 0.4f, 0.9f, 1.0f});
		//Renderer2D::DrawQuad({ 0.0f, -0.5f }, 1.0f, 0.2f, { 0.0f, 0.4f, 0.9f, 1.0f });
		//
		//Renderer2D::DrawRotatedQuad({ 0.0f,0.0f}, 1.5f, 0.2f, glm::radians(m_Time * 7.0f), {1.0f, 0.5f,0.4f, 0.5f});
		//Renderer2D::DrawRotatedQuad({ 0.0f,0.0f}, 1.5f, 0.2f, glm::radians(-m_Time * 7.0f), {1.0f, 0.5f,0.4f, 0.5f});
		//
		//Renderer2D::DrawCircle({ 0.0f,0.0f }, 0.1f, Animate(0.2f), 0.05f, {Animate(1.0f), 0.8f, 0.9f, 0.5f});
		//Renderer2D::DrawCircle({ 0.5f,0.7f }, 0.1f, Animate(0.2f), 0.05f, {Animate(1.0f), 0.8f, 0.9f, 0.5f});
		//Renderer2D::DrawCircle({ -0.7f,0.2f }, 0.1f, Animate(0.2f), 0.05f, {Animate(1.0f), 0.8f, 0.9f, 0.5f});
		//
		//Renderer2D::DrawLine({ 0.0f,0.0f }, { 0.5f, 0.7f }, m_Time * 0.001f, { 1.0f, 1.0f, 0.0f, 0.7f });
		//Renderer2D::DrawLine({ 0.5f,0.7f }, { -0.7f, 0.2f }, m_Time * 0.001f, { 1.0f, 1.0f, 0.0f, 0.7f });
		//Renderer2D::DrawLine({ -0.7f,0.2f }, { 0.0f, 0.0f }, m_Time * 0.001f, { 1.0f, 1.0f, 0.0f, 0.7f });

		Renderer2D::DrawRect({ 0.0f, 0.0f }, 0.5f, 0.3f, 0.06f, { 1.0f, 1.0f, 0.8f, 1.0f });
		Renderer2D::DrawRotatedQuad({ 1.0f, 0.5f }, 0.5f, 0.3f, 0.8f * Animate(0.5f), { 1.0f, 0.7f, 0.0f, 1.0f });
		Renderer2D::DrawRotatedRect({ 1.0f, 0.5f }, 0.5f, 0.3f, 0.05f, 0.8f * Animate(0.5f), { 1.0f, 0.1f, 0.2f, 1.0f });

		Renderer2D::EndScene();

		Renderer::EndRenderPass();
	}

	void RenderingTestLayer::OnUIRender()
	{
	}

	void RenderingTestLayer::OnEvent(Event& event)
	{
	}

	void RenderingTestLayer::OnDetach()
	{
	}
}
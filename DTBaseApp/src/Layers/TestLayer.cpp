#include "TestLayer.h"
#include "Core/Application.h"
#include "Renderer/Renderer.h"
#include "Renderer/Renderer2D.h"
#include "imgui.h"
#include "Core/Input.h"

namespace DT
{
	void TestLayer::OnAttach()
	{
		FramebufferSpecification geoFramebufferSpec{};
		geoFramebufferSpec.Format = ImageFormat::RGBA8;
		geoFramebufferSpec.SwapchainTarget = true;
		m_GeoFramebuffer = CreateRef<Framebuffer>(geoFramebufferSpec);

		Renderer2D::SetTargetFramebuffer(m_GeoFramebuffer);
	}

	void TestLayer::OnUpdate(float dt)
	{
	}

	void TestLayer::OnEvent(Event& event)
	{
	}

	void TestLayer::OnDetach()
	{
	}

	void TestLayer::OnRender()
	{
		Renderer2D::BeginScene();
		Renderer2D::DrawCircle({ 0.0f, 0.0f }, 0.7f, 0.1f, 0.5f, { 0.7f, 0.2f, 0.2f, 1.0f });
		Renderer2D::EndScene();
	}

	void TestLayer::OnUIRender()
	{
		ImGui::Begin("Test");
		//ImGui::Image(m_GeoFramebuffer->GetImage()->GetSRV(), { 160.0f, 90.0f });
		ImGui::End();
	}
}
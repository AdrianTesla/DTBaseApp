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

		m_Texture = CreateRef<Texture2D>("assets/textures/M_FloorTiles1_Inst_0_BaseColor.png");
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

		Renderer2D::DrawQuad({ 0.5f, 0.5f }, 0.2f, 0.2f, { 0.0f, 0.4f, 0.9f, 1.0f });
		Renderer2D::DrawQuad({ -0.5f, 0.5f }, 0.2f, 0.5f * Animate(0.2f), {0.0f, 0.4f, 0.9f, 1.0f});
		Renderer2D::DrawQuad({ 0.0f, -0.5f }, 1.0f, 0.2f, { 0.0f, 0.4f, 0.9f, 1.0f });
		
		Renderer2D::DrawRotatedQuad({ 0.0f,0.0f}, 1.5f, 0.2f, glm::radians(m_Time * 7.0f), {1.0f, 0.5f,0.4f, 0.5f});
		Renderer2D::DrawRotatedQuad({ 0.0f,0.0f}, 1.5f, 0.2f, glm::radians(-m_Time * 7.0f), {1.0f, 0.5f,0.4f, 0.5f});
		
		Renderer2D::DrawCircle({ 0.0f,0.0f }, 0.1f, Animate(0.2f), 0.05f, {Animate(1.0f), 0.8f, 0.9f, 0.5f});
		Renderer2D::DrawCircle({ 0.5f,0.7f }, 0.1f, Animate(0.2f), 0.05f, {Animate(1.0f), 0.8f, 0.9f, 0.5f});
		Renderer2D::DrawCircle({ -0.7f,0.2f }, 0.1f, Animate(0.2f), 0.05f, {Animate(1.0f), 0.8f, 0.9f, 0.5f});
		
		Renderer2D::DrawLine({ 0.0f,0.0f }, { 0.5f, 0.7f }, m_Time * 0.001f, { 1.0f, 1.0f, 0.0f, 0.7f });
		Renderer2D::DrawLine({ 0.5f,0.7f }, { -0.7f, 0.2f }, m_Time * 0.001f, { 1.0f, 1.0f, 0.0f, 0.7f });
		Renderer2D::DrawLine({ -0.7f,0.2f }, { 0.0f, 0.0f }, m_Time * 0.001f, { 1.0f, 1.0f, 0.0f, 0.7f });

		Renderer2D::DrawRect({0.0f, 0.0f}, m_Width, m_Height, m_Thickness, { 1.0f, 1.0f, 0.8f, 1.0f });
		Renderer2D::DrawRotatedQuad({ 1.0f, 0.5f }, 0.5f, 0.3f, m_Angle, { 1.0f, 0.7f, 0.0f, 1.0f });
		Renderer2D::DrawRotatedRect({ 1.0f, 0.5f }, 0.5f, 0.3f, m_Thickness, m_Angle, { 1.0f, 0.1f, 0.2f, 1.0f });
		Renderer2D::DrawCircle(m_Position, m_Radius, m_CircleThickness, m_Fade, m_Color);
		Renderer2D::DrawTexturedQuad({ 0.0f, 0.0f }, m_Width, m_Height, m_Texture, m_Tiling, m_Color);
		Renderer2D::DrawRotatedTexQuad({ 0.5f,0.5f }, m_Width, m_Height, m_Texture, m_Tiling, Animate(1.0f), m_Color);

		Renderer2D::EndScene();

		Renderer::EndRenderPass();
	}

	void RenderingTestLayer::OnUIRender()
	{
		ImGui::Begin("test");
		ImGui::Button("Premimi");
		ImGui::ColorButton("Colore di merda di imgui", { 0.2f, 0.3f, 0.5f, 0.7f });
		ImGui::Separator();
		ImGui::SliderFloat("Thickness", &m_Thickness, 0.0f, 0.1f);
		ImGui::SliderAngle("Angle", &m_Angle, 0.0f, 180.0f);
		ImGui::SliderFloat("Width", &m_Width, 0.0f, 1.5f);
		ImGui::SliderFloat("Height", &m_Height, 0.0f, 1.5f);
		ImGui::Separator();
		ImGui::SliderFloat("Circle thickness", &m_CircleThickness, 0.0f, 1.0f);
		ImGui::SliderFloat("Fade", &m_Fade, 0.0f, 1.0f);
		ImGui::SliderFloat2("Position", glm::value_ptr(m_Position), -1.0f, 1.0f);
		ImGui::SliderFloat("Radius", &m_Radius, 0.0f, 1.0f);
		ImGui::Separator();
		ImGui::ColorEdit4("Color", glm::value_ptr(m_Color), ImGuiColorEditFlags_PickerHueWheel);
		ImGui::Separator();
		ImGui::SliderFloat("Tiling", &m_Tiling, 0.0f, 5.0f);
		ImGui::End();
	}

	void RenderingTestLayer::OnEvent(Event& event)
	{
	}

	void RenderingTestLayer::OnDetach()
	{
	}
}
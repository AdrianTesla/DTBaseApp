#include "BloomLayer.h"
#include "Core/Application.h"
#include "Renderer/Renderer.h"
#include "Renderer/Renderer2D.h"
#include "imgui.h"
#include "Core/Input.h"

namespace DT
{
	void BloomLayer::OnAttach()
	{
		FramebufferSpecification geoFramebufferSpec{};
		geoFramebufferSpec.SwapchainTarget = false;
		geoFramebufferSpec.Format = ImageFormat::RGBA16F;
		m_GeoFramebuffer = CreateRef<Framebuffer>(geoFramebufferSpec);

		FramebufferSpecification screenFramebufferSpec{};
		screenFramebufferSpec.SwapchainTarget = true;
		m_ScreenFramebuffer = CreateRef<Framebuffer>(screenFramebufferSpec);

		Renderer2D::SetTargetFramebuffer(m_GeoFramebuffer);
	}

	void BloomLayer::OnUpdate(float dt)
	{
		m_Time += dt;
	}

	void BloomLayer::OnEvent(Event& event)
	{
	}

	void BloomLayer::OnDetach()
	{
	}

	void BloomLayer::OnRender()
	{
		m_GeoFramebuffer->ClearAttachment({ 0.0f, 0.0f, 0.0f, 1.0f });
		m_ScreenFramebuffer->ClearAttachment({ 0.0f, 0.0f, 0.0f, 1.0f });
		Renderer2D::BeginScene();
		glm::vec4 color = m_Color;
		color.r *= m_Emission;
		color.g *= m_Emission;
		color.b *= m_Emission;
		Renderer2D::DrawRotatedQuad({ 0.0f, 0.0f }, 0.5f, 0.5f, m_Time, color);
		Renderer2D::EndScene();

		m_BloomProcessor.Execute(m_GeoFramebuffer->GetImage(), m_ScreenFramebuffer);
	}

	void BloomLayer::OnUIRender()
	{
		BloomSettings& settings = m_BloomProcessor.GetSettings();

		ImGui::Begin("Bloom");
		ImGui::ColorEdit4("Color", glm::value_ptr(m_Color), ImGuiColorEditFlags_PickerHueWheel);
		ImGui::DragFloat("Emission", &m_Emission, 0.005f, 0.0f, 100.0f);
		ImGui::SliderFloat("Radius", &settings.Radius, 0.0f, 10.0f);
		ImGui::SliderFloat("Threshold", &settings.Threshold, 0.0f, 10.0f);
		ImGui::SliderFloat("Knee", &settings.Knee, 0.0f, 1.0f);
		ImGui::SliderFloat("Clamp", &settings.Clamp, 0.0f, 1000.0f);
		ImGui::SliderFloat("Intensity", &settings.Intensity, 0.0f, 0.1f);
		ImGui::End();
	}
}
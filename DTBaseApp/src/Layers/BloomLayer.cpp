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
		geoFramebufferSpec.SwapchainTarget = true;
		m_GeoFramebuffer = CreateRef<Framebuffer>(geoFramebufferSpec);

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
		Renderer2D::BeginScene();
		Renderer2D::DrawRotatedQuad({ 0.0f, 0.0f }, 0.5f, 0.5f, m_Time, m_Color * m_Emission);
		Renderer2D::EndScene();
	}

	void BloomLayer::OnUIRender()
	{
		ImGui::Begin("Bloom");
		ImGui::ColorEdit4("Color", glm::value_ptr(m_Color), ImGuiColorEditFlags_PickerHueWheel);
		ImGui::DragFloat("Emission", &m_Emission, 0.005f, 0.0f, 100.0f);
		ImGui::End();
	}
}
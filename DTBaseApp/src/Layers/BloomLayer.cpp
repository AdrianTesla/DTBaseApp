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

		FramebufferSpecification stage0FramebufferSpec{};
		stage0FramebufferSpec.SwapchainTarget = false;
		stage0FramebufferSpec.Format = ImageFormat::RGBA16F;
		stage0FramebufferSpec.Scale = 0.5f;
		m_BloomStage0 = CreateRef<Framebuffer>(stage0FramebufferSpec);

		PipelineSpecification downscalePipelineSpec{};
		downscalePipelineSpec.BlendingEnabled = false;
		downscalePipelineSpec.VertexShaderPath = "DownscaleVS.cso";
		downscalePipelineSpec.PixelShaderPath = "DownscalePS.cso";

		RenderPassSpecification stage0PassSpec{};
		stage0PassSpec.TargetFramebuffer = m_BloomStage0;
		stage0PassSpec.Pipeline = CreateRef<Pipeline>(downscalePipelineSpec);
		m_BloomPass0 = CreateRef<RenderPass>(stage0PassSpec);
		m_BloomPass0->SetInput("Previous Stage", m_GeoFramebuffer->GetImage(), 0u);

		m_Sampler = CreateRef<Sampler>(true);
		m_Sampler->Bind(0);

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

		//begin bloom post processing 
		Renderer::BeginRenderPass(m_BloomPass0, true);
		Renderer::DrawFullscreenQuad();
		Renderer::EndRenderPass();
	}

	void BloomLayer::OnUIRender()
	{
		ImGui::Begin("Bloom");
		ImGui::ColorEdit4("Color", glm::value_ptr(m_Color), ImGuiColorEditFlags_PickerHueWheel);
		ImGui::DragFloat("Emission", &m_Emission, 0.005f, 0.0f, 100.0f);
		ImGui::End();

		std::string title = std::format("GeoFramebuffer ({}, {})", m_GeoFramebuffer->GetWidth(), m_GeoFramebuffer->GetHeight());
		ImGui::Begin(title.c_str());
		float w = m_GeoFramebuffer->GetWidth() * 0.5f;
		float h = m_GeoFramebuffer->GetHeight() * 0.5f;
		ImGui::Image(m_GeoFramebuffer->GetImage()->GetSRV(), { w, h });
		ImGui::End();

		title = std::format("BloomStage0 ({}, {})", m_BloomStage0->GetWidth(), m_BloomStage0->GetHeight());
		ImGui::Begin(title.c_str());
		w = m_BloomStage0->GetWidth() * 2.0f;
		h = m_BloomStage0->GetHeight() * 2.0f;
		ImGui::Image(m_BloomStage0->GetImage()->GetSRV(), { w, h });
		ImGui::End();
	}
}
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

		InitBloom();

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
		ExecuteBloom();
	}

	void BloomLayer::OnUIRender()
	{
		ImGui::Begin("Bloom");
		ImGui::ColorEdit4("Color", glm::value_ptr(m_Color), ImGuiColorEditFlags_PickerHueWheel);
		ImGui::DragFloat("Emission", &m_Emission, 0.005f, 0.0f, 100.0f);
		ImGui::SliderInt("Bloom Stage", &m_StageIndex, 0, m_StageCount - 1);
		ImGui::End();

		std::string title = std::format("GeoFramebuffer ({}, {})", m_GeoFramebuffer->GetWidth(), m_GeoFramebuffer->GetHeight());
		ImGui::Begin(title.c_str());
		float w = m_GeoFramebuffer->GetWidth() * 0.5f;
		float h = m_GeoFramebuffer->GetHeight() * 0.5f;
		ImGui::Image(m_GeoFramebuffer->GetImage()->GetSRV(), { w, h });
		ImGui::End();

		title = std::format("BloomStage{} ({}, {})",m_StageIndex, m_BloomStages[m_StageIndex]->GetWidth(), m_BloomStages[m_StageIndex]->GetHeight());
		ImGui::Begin(title.c_str());
		w = m_BloomStages[m_StageIndex]->GetWidth() * std::pow(2.0f, m_StageIndex);
		h = m_BloomStages[m_StageIndex]->GetHeight() * std::pow(2.0f, m_StageIndex);
		ImGui::Image(m_BloomStages[m_StageIndex]->GetImage()->GetSRV(), {w, h});
		ImGui::End();
	}

	void BloomLayer::InitBloom()
	{
		//Create downscale pipeline 
		PipelineSpecification downscalePipelineSpec{};
		downscalePipelineSpec.BlendingEnabled = false;
		downscalePipelineSpec.VertexShaderPath = "DownscaleVS.cso";
		downscalePipelineSpec.PixelShaderPath = "DownscalePS.cso";

		//Create all the bloom stages 
		float scale = 0.5f;
		uint32 iterations = std::log2((float)m_GeoFramebuffer->GetHeight()) - 1.0f;
		for (uint32 i = 0u; i < std::min(iterations,m_StageCount); i++)
		{
			FramebufferSpecification framebufferSpec{};
			framebufferSpec.SwapchainTarget = false;
			framebufferSpec.Format = ImageFormat::RGBA16F;
			framebufferSpec.Scale = scale;
			m_BloomStages[i] = CreateRef<Framebuffer>(framebufferSpec);
			scale = scale / 2.0f;
		}

		//create render passes for downscale steps
		for (uint32 i = 0u; i < m_StageCount; i++)
		{
			RenderPassSpecification passSpec{};
			passSpec.TargetFramebuffer = m_BloomStages[i];
			passSpec.Pipeline = CreateRef<Pipeline>(downscalePipelineSpec);
			m_BloomDownscalePasses[i] = CreateRef<RenderPass>(passSpec);
		}

		m_BloomDownscalePasses[0]->SetInput("Pass 0", m_GeoFramebuffer->GetImage(), 0u);
		for (uint32 i = 1u; i < m_StageCount; i++)
		{
			m_BloomDownscalePasses[i]->SetInput(std::format("Pass {}", i).c_str(), m_BloomStages[i - 1]->GetImage(), 0u);
		}

		m_Sampler = CreateRef<Sampler>(true);
		m_Sampler->Bind(0);
	}

	void BloomLayer::ExecuteBloom()
	{
		for (uint32 i = 0u; i < m_StageCount; i++)
		{
			Renderer::BeginRenderPass(m_BloomDownscalePasses[i], false);
			Renderer::DrawFullscreenQuad();
			Renderer::EndRenderPass();
		}
	}
}
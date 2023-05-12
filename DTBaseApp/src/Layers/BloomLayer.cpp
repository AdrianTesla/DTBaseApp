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

		//std::string title = std::format("Prefilter framebuffer ({}, {})", m_PrefilterFramebuffer->GetWidth(), m_PrefilterFramebuffer->GetHeight());
		//ImGui::Begin(title.c_str());
		//float w = m_PrefilterFramebuffer->GetWidth() * 0.5f;
		//float h = m_PrefilterFramebuffer->GetHeight() * 0.5f;
		//ImGui::Image(m_PrefilterFramebuffer->GetImage()->GetSRV(), { w, h });
		//ImGui::End();
		//
		//title = std::format("BloomStage{} ({}, {})",m_StageIndex, m_BloomStages[m_StageIndex]->GetWidth(), m_BloomStages[m_StageIndex]->GetHeight());
		//ImGui::Begin(title.c_str());
		//float w = m_BloomStages[m_StageIndex]->GetWidth() * std::pow(2.0f, m_StageIndex);
		//float h = m_BloomStages[m_StageIndex]->GetHeight() * std::pow(2.0f, m_StageIndex);
		//ImGui::Image(m_BloomStages[m_StageIndex]->GetImage()->GetSRV(), {w, h});
		//ImGui::End();
	}

	//void BloomLayer::InitBloom()
	//{
	//	/* determine the iteration count */
	//	const float minDim = (float)std::min(m_GeoFramebuffer->GetWidth(), m_GeoFramebuffer->GetHeight());
	//	const float maxIter = (m_Radius - 8.0f) + std::log(minDim) / std::log(2.0f);
	//	const int maxIterInt = m_StageCount = (int)maxIter;
	//	m_StageCount = 8u;

	//	//Create prefilter pipeline
	//	PipelineSpecification prefilterPipelineSpec{};
	//	prefilterPipelineSpec.BlendingMode = BlendingMode::None;
	//	prefilterPipelineSpec.VertexShaderPath = "PrefilterVS.cso";
	//	prefilterPipelineSpec.PixelShaderPath = "PrefilterPS.cso";

	//	//Create downscale pipeline 
	//	PipelineSpecification downscalePipelineSpec{};
	//	downscalePipelineSpec.BlendingMode = BlendingMode::None;
	//	downscalePipelineSpec.VertexShaderPath = "DownscaleVS.cso";
	//	downscalePipelineSpec.PixelShaderPath = "DownscalePS.cso";

	//	//Create upscale pipeline
	//	PipelineSpecification upscalePipelineSpec{};
	//	upscalePipelineSpec.BlendingMode = BlendingMode::Additive;
	//	upscalePipelineSpec.VertexShaderPath = "UpscaleVS.cso";
	//	upscalePipelineSpec.PixelShaderPath = "UpscalePS.cso";

	//	//Create prefilter framebuffer 
	//	FramebufferSpecification prefilterFramebufferSpec{};
	//	prefilterFramebufferSpec.Format = ImageFormat::R11G11B10F;
	//	prefilterFramebufferSpec.Scale = 1.0f;
	//	prefilterFramebufferSpec.SwapchainTarget = false;
	//	m_PrefilterFramebuffer = CreateRef<Framebuffer>(prefilterFramebufferSpec);

	//	m_PrefilterUB = CreateRef<UniformBuffer>(sizeof(PrefilterUB));
	//	m_UpscaleUB = CreateRef<UniformBuffer>(sizeof(UpscaleUB));
	//	m_CombineUB = CreateRef<UniformBuffer>(sizeof(CombineUB));

	//	//Create prefilter renderpass
	//	RenderPassSpecification prefilterPassSpec{};
	//	prefilterPassSpec.Pipeline = CreateRef<Pipeline>(prefilterPipelineSpec);
	//	prefilterPassSpec.TargetFramebuffer = m_PrefilterFramebuffer;
	//	m_PrefilterPass = CreateRef<RenderPass>(prefilterPassSpec);
	//	m_PrefilterPass->SetInput("Geometry framebuffer", m_GeoFramebuffer->GetImage(), 0u);
	//	m_PrefilterPass->SetInput("Input parameters", m_PrefilterUB, 0u);

	//	//Create all the bloom stages 
	//	float scale = 0.5f;
	//	for (uint32 i = 0u; i < m_StageCount; i++)
	//	{
	//		FramebufferSpecification framebufferSpec{};
	//		framebufferSpec.SwapchainTarget = false;
	//		framebufferSpec.Format = ImageFormat::RGBA16F;
	//		framebufferSpec.Scale = scale;
	//		m_BloomStages[i] = CreateRef<Framebuffer>(framebufferSpec);
	//		scale = scale / 2.0f;
	//	}

	//	//Create render passes for downscale steps
	//	for (uint32 i = 0u; i < m_StageCount; i++)
	//	{
	//		RenderPassSpecification passSpec{};
	//		passSpec.TargetFramebuffer = m_BloomStages[i];
	//		passSpec.Pipeline = CreateRef<Pipeline>(downscalePipelineSpec);
	//		m_BloomDownscalePasses[i] = CreateRef<RenderPass>(passSpec);
	//	}

	//	m_BloomDownscalePasses[0]->SetInput("Downpass 0", m_PrefilterFramebuffer->GetImage(), 0u);
	//	for (uint32 i = 1u; i < m_StageCount; i++)
	//	{
	//		m_BloomDownscalePasses[i]->SetInput(std::format("Downpass {}", i).c_str(), m_BloomStages[i - 1]->GetImage(), 0u);
	//	}

	//	//Create render passes for upscale steps
	//	for (uint32 i = 0u; i < m_StageCount - 1u; i++)
	//	{
	//		RenderPassSpecification passSpec{};
	//		passSpec.TargetFramebuffer = m_BloomStages[m_StageCount - 2u - i];
	//		passSpec.Pipeline = CreateRef<Pipeline>(upscalePipelineSpec);
	//		m_BloomUpscalePasses[i] = CreateRef<RenderPass>(passSpec);
	//	}

	//	for (uint32 i = 0u; i < m_StageCount - 1u; i++)
	//	{
	//		m_BloomUpscalePasses[i]->SetInput(std::format("UpPass {}", i).c_str(), m_BloomStages[m_StageCount - 1u - i]->GetImage(), 0u);
	//		m_BloomUpscalePasses[i]->SetInput("Scale {}", m_UpscaleUB, 0u);
	//	}

	//	//Create the combine pipeline
	//	PipelineSpecification combinePipelineSpec{};
	//	combinePipelineSpec.BlendingMode = BlendingMode::None;
	//	combinePipelineSpec.VertexShaderPath = "CombineVS.cso";
	//	combinePipelineSpec.PixelShaderPath = "CombinePS.cso";

	//	//Create the combine render pass
	//	RenderPassSpecification combinePassSpec{};
	//	combinePassSpec.Pipeline = CreateRef<Pipeline>(combinePipelineSpec);
	//	combinePassSpec.TargetFramebuffer = m_ScreenFramebuffer;
	//	combinePassSpec.ClearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
	//	m_CombinePass = CreateRef<RenderPass>(combinePassSpec);
	//	m_CombinePass->SetInput("Original image", m_GeoFramebuffer->GetImage(), 0u);
	//	m_CombinePass->SetInput("Bloom Stage 0", m_BloomStages[0]->GetImage(), 1u);
	//	m_CombinePass->SetInput("Combine parameters", m_CombineUB, 0u);

	//	m_Sampler = CreateRef<Sampler>(true);
	//	m_Sampler->Bind(0);
	//}

	//void BloomLayer::ExecuteBloom()
	//{
	//	m_PrefilterUB->SetData(&s_PrefilterUB, sizeof(PrefilterUB));

	//	/* determine the iteration count */
	//	const float minDim = (float)std::min(m_GeoFramebuffer->GetWidth(), m_GeoFramebuffer->GetHeight());
	//	const float maxIter = (m_Radius - 8.0f) + std::log(minDim) / std::log(2);
	//	const int maxIterInt = m_StageCount = (int)maxIter;
	//	m_StageCount = std::clamp(m_StageCount, 1u, 8u);
	//	m_StageCount = 8u;

	//	//s_UpscaleUB.SampleScale = 0.5f + maxIter - (float)maxIterInt;
	//	s_PrefilterUB.CurveThreshold[0] = s_PrefilterUB.Threshold - s_PrefilterUB.Knee;
	//	s_PrefilterUB.CurveThreshold[1] = s_PrefilterUB.Knee * 2.0f;
	//	s_PrefilterUB.CurveThreshold[2] = 0.25f / std::max(1e-5f, s_PrefilterUB.Knee);
	//	s_PrefilterUB.CurveThreshold[3] = s_PrefilterUB.Threshold;
	//	m_UpscaleUB->SetData(&s_UpscaleUB, sizeof(s_UpscaleUB));

	//	//Execute prefilter pass
	//	Renderer::BeginRenderPass(m_PrefilterPass, false);
	//	Renderer::DrawFullscreenQuad();
	//	Renderer::EndRenderPass();

	//	//Execute downscale passes
	//	for (uint32 i = 0u; i < m_StageCount; i++)
	//	{
	//		Renderer::BeginRenderPass(m_BloomDownscalePasses[i], false);
	//		Renderer::DrawFullscreenQuad();
	//		Renderer::EndRenderPass();
	//	}

	//	//Execute upscale passes
	//	for (uint32 i = 0u; i < m_StageCount - 1u; i++)
	//	{
	//		Renderer::BeginRenderPass(m_BloomUpscalePasses[i], false);
	//		Renderer::DrawFullscreenQuad();
	//		Renderer::EndRenderPass();
	//	}

	//	//Execute combine pass
	//	s_CombineUB.UpsampleScale = s_UpscaleUB.SampleScale;
	//	m_CombineUB->SetData(&s_CombineUB, sizeof(s_CombineUB));
	//	Renderer::BeginRenderPass(m_CombinePass, true);
	//	Renderer::DrawFullscreenQuad();
	//	Renderer::EndRenderPass();
	//}
}
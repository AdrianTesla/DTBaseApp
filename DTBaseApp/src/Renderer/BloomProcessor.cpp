#include "BloomProcessor.h"
#include "Renderer.h"

namespace DT
{
	BloomProcessor::BloomProcessor()
	{
		//Create prefilter pipeline
		PipelineSpecification prefilterPipelineSpec{};
		prefilterPipelineSpec.BlendingMode = BlendingMode::None;
		prefilterPipelineSpec.VertexShaderPath = "PrefilterVS.cso";
		prefilterPipelineSpec.PixelShaderPath = "PrefilterPS.cso";
		m_PrefilterPipeline = Pipeline::Create(prefilterPipelineSpec);

		//Create downscale pipeline 
		PipelineSpecification downscalePipelineSpec{};
		downscalePipelineSpec.BlendingMode = BlendingMode::None;
		downscalePipelineSpec.VertexShaderPath = "DownscaleVS.cso";
		downscalePipelineSpec.PixelShaderPath = "DownscalePS.cso";
		m_DownscalePipeline = Pipeline::Create(downscalePipelineSpec);

		//Create upscale pipeline
		PipelineSpecification upscalePipelineSpec{};
		upscalePipelineSpec.BlendingMode = BlendingMode::Additive;
		upscalePipelineSpec.VertexShaderPath = "UpscaleVS.cso";
		upscalePipelineSpec.PixelShaderPath = "UpscalePS.cso";
		m_UpscalePipeline = Pipeline::Create(upscalePipelineSpec);

		//Create the combine pipeline
		PipelineSpecification combinePipelineSpec{};
		combinePipelineSpec.BlendingMode = BlendingMode::None;
		combinePipelineSpec.VertexShaderPath = "CombineVS.cso";
		combinePipelineSpec.PixelShaderPath = "CombinePS.cso";
		m_CombinePipeline = Pipeline::Create(combinePipelineSpec);

		//Create uniform buffers
		m_PrefilterUB = UniformBuffer::Create(sizeof(PrefilterUB));
		m_DownscaleUB = UniformBuffer::Create(sizeof(DownscaleUB));
		m_UpscaleUB = UniformBuffer::Create(sizeof(UpscaleUB));
		m_CombineUB = UniformBuffer::Create(sizeof(CombineUB));

		//Create prefilter framebuffer 
		FramebufferSpecification prefilterFramebufferSpec{};
		prefilterFramebufferSpec.SwapchainTarget = false;
		prefilterFramebufferSpec.Format = ImageFormat::R11G11B10F;

		//Create the prefilter render pass
		RenderPassSpecification prefilterPassSpec{};
		prefilterPassSpec.Pipeline = m_PrefilterPipeline;
		prefilterPassSpec.TargetFramebuffer = Framebuffer::Create(prefilterFramebufferSpec);
		m_PrefilterPass = RenderPass::Create(prefilterPassSpec);

		//Create combine render pass
		RenderPassSpecification combinePassSpec{};
		combinePassSpec.Pipeline = m_CombinePipeline;
		m_CombinePass = RenderPass::Create(combinePassSpec);

		m_Sampler = CreateRef<Sampler>(true);
	}

	void BloomProcessor::Resize(uint32 width, uint32 height)
	{
		m_Width = width;
		m_Height = height;
		uint32 minSize = std::min(width, height);
		m_MaxStages = 0u;
		for (uint32 s = minSize; s > 1u; s /= 2u)
			m_MaxStages++;

		//Create bloom stages 
		float scale = 0.5f;
		for (uint32 i = 0u; i < m_MaxStages; i++)
		{
			FramebufferSpecification framebufferSpec{};
			framebufferSpec.SwapchainTarget = false;
			framebufferSpec.Format = ImageFormat::R11G11B10F;
			framebufferSpec.Scale = scale;
			m_Stages[i] = Framebuffer::Create(framebufferSpec);
			scale = scale / 2.0f;
		}

		//Create downscale render passes
		for (uint32 i = 0u; i < m_MaxStages; i++)
		{
			RenderPassSpecification passSpec{};
			passSpec.Pipeline = m_DownscalePipeline;
			m_DownscalePasses[i] = RenderPass::Create(passSpec);
		}

		//Create upscale render passes
		for (uint32 i = 0u; i < m_MaxStages - 1u; i++)
		{
			RenderPassSpecification passSpec{};
			passSpec.Pipeline = m_UpscalePipeline;
			m_UpscalePasses[i] = RenderPass::Create(passSpec);
		}
	}

	BloomSettings& BloomProcessor::GetSettings()
	{
		return m_Settings;
	}

	void BloomProcessor::Execute(const Ref<Image2D>& input, const Ref<Framebuffer>& output)
	{
		uint32 width = input->GetWidth();
		uint32 height = input->GetHeight();
		if (m_Width != width || m_Height != height)
			Resize(width, height);

		m_Sampler->Bind(0u);

		m_PrefilterUBData.TexelSize = 1.0f / glm::vec2((float)width, (float)height);
		m_PrefilterUBData.ClampIntensity = m_Settings.Clamp;
		m_PrefilterUBData.Knee = m_Settings.Knee;
		m_PrefilterUBData.CurveThreshold.x = m_Settings.Threshold - m_Settings.Knee;
		m_PrefilterUBData.CurveThreshold.y = m_Settings.Knee * 2.0f;
		m_PrefilterUBData.CurveThreshold.z = 0.25f / std::max(m_Settings.Knee, 0.00001f);
		m_PrefilterUBData.CurveThreshold.w = m_Settings.Threshold;
		m_PrefilterUB->SetData(&m_PrefilterUBData, sizeof(PrefilterUB));

		m_PrefilterPass->SetInput("Input image", input, 0u);
		m_PrefilterPass->SetInput("Prefilter UB", m_PrefilterUB, 0u);

		uint32 minSize = std::min(width, height);
		float logh = std::log2f((float)minSize) + m_Settings.Radius - 8.0f;
		uint32 logi = (uint32)logh;
		m_Iterations = std::clamp(logi, 1u, m_MaxStages);

		//Execute prefilter render pass
		Renderer::BeginRenderPass(m_PrefilterPass, false);
		Renderer::DrawFullscreenQuad();
		Renderer::EndRenderPass();

		float prefilterWidth = m_PrefilterPass->GetOutput()->GetImage()->GetWidth();
		float prefilterHeight = m_PrefilterPass->GetOutput()->GetImage()->GetHeight();
		m_DownscaleUBData.TexelSize = 1.0f / glm::vec2(prefilterWidth, prefilterHeight);
		m_DownscaleUB->SetData(&m_DownscaleUBData, sizeof(DownscaleUB));

		m_DownscalePasses[0]->SetInput("Prefilter output", m_PrefilterPass->GetOutput()->GetImage(), 0u);
		m_DownscalePasses[0]->SetInput("Downscale UB", m_DownscaleUB, 0u);
		m_DownscalePasses[0]->GetSpecification().TargetFramebuffer = m_Stages[0];

		//Execute downscale render passes
		Renderer::BeginRenderPass(m_DownscalePasses[0], false);
		Renderer::DrawFullscreenQuad();
		Renderer::EndRenderPass();

		for (uint32 i = 1u; i < m_Iterations; i++)
		{
			float previousWidth = m_Stages[i - 1]->GetImage()->GetWidth();
			float previousHeight = m_Stages[i - 1]->GetImage()->GetHeight();
			m_DownscaleUBData.TexelSize = 1.0f / glm::vec2(previousWidth, previousHeight);
			m_DownscaleUB->SetData(&m_DownscaleUBData, sizeof(DownscaleUB));

			m_DownscalePasses[i]->SetInput(std::format("Stage {}", i), m_Stages[i - 1]->GetImage(), 0u);
			m_DownscalePasses[i]->SetInput(std::format("Downscale UB {}", i), m_DownscaleUB, 0u);
			m_DownscalePasses[i]->GetSpecification().TargetFramebuffer = m_Stages[i];

			Renderer::BeginRenderPass(m_DownscalePasses[i], false);
			Renderer::DrawFullscreenQuad();
			Renderer::EndRenderPass();
		}

		m_UpscaleUBData.SampleScale = 0.5f + logh - (float)logi;

		//Execute upscale passes
		for (uint32 i = 0u; i < m_Iterations - 1u; i++)
		{
			float previousWidth = m_Stages[m_Iterations - 1 - i]->GetImage()->GetWidth();
			float previousHeight = m_Stages[m_Iterations - 1 - i]->GetImage()->GetHeight();
			m_UpscaleUBData.TexelSize = 1.0f / glm::vec2(previousWidth, previousHeight);
			m_UpscaleUB->SetData(&m_UpscaleUBData, sizeof(UpscaleUB));

			m_UpscalePasses[i]->SetInput(std::format("input stage {}", i), m_Stages[m_Iterations - 1 - i]->GetImage(), 0u);
			m_UpscalePasses[i]->SetInput(std::format("Upscale UB {}", i), m_UpscaleUB, 0u);
			m_UpscalePasses[i]->GetSpecification().TargetFramebuffer = m_Stages[m_Iterations - 2 - i];

			Renderer::BeginRenderPass(m_UpscalePasses[i], false);
			Renderer::DrawFullscreenQuad();
			Renderer::EndRenderPass();
		}

		float lastStageWidth = m_Stages[0]->GetImage()->GetWidth();
		float lastStageHeight = m_Stages[0]->GetImage()->GetHeight();
		m_CombineUBData.TexelSize = 1.0f / glm::vec2(lastStageWidth, lastStageHeight);
		m_CombineUBData.BloomIntensity = m_Settings.Intensity;
		m_CombineUBData.UpsampleScale = m_UpscaleUBData.SampleScale;
		m_CombineUB->SetData(&m_CombineUBData, sizeof(CombineUB));

		m_CombinePass->SetInput("Original Image", input, 0u);
		m_CombinePass->SetInput("Bloomed Image", m_Stages[0]->GetImage(), 1u);
		m_CombinePass->SetInput("Combine UB", m_CombineUB, 0u);
		m_CombinePass->GetSpecification().TargetFramebuffer = output;

		//Execute combine pass
		Renderer::BeginRenderPass(m_CombinePass, false);
		Renderer::DrawFullscreenQuad();
		Renderer::EndRenderPass();	
	}
}

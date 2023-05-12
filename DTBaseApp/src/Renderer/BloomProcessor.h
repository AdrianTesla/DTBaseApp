#pragma once
#include "Framebuffer.h"
#include "Pipeline.h"
#include "RenderPass.h"
#include "Sampler.h"

namespace DT
{
	struct BloomSettings
	{
		float Intensity = 0.05f;
		float Threshold = 0.8f;
		float Knee = 0.5f;
		float Radius = 6.5f;
		float Clamp = 0.0f;
	};

	class BloomProcessor
	{
	public:
		BloomProcessor();
		void Execute(const Ref<Image2D>& input, const Ref<Framebuffer>& output);
		BloomSettings& GetSettings();
	private:
		void Resize(uint32 width, uint32 height);
	private:
		struct PrefilterUB
		{
			glm::vec4 CurveThreshold;
			float Knee = 0.5f;
			float ClampIntensity = 0.0f;
			float pad[2];
		};

		struct UpscaleUB
		{
			float SampleScale = 0.0f;
			float pad[3];
		};

		struct CombineUB
		{
			float BloomIntensity = 0.05f;
			float UpsampleScale = 0.0f;
			float pad[2];
		};
		PrefilterUB m_PrefilterUBData;
		UpscaleUB m_UpscaleUBData;
		CombineUB m_CombineUBData;
	private:
		Ref<Pipeline> m_PrefilterPipeline;
		Ref<Pipeline> m_DownscalePipeline;
		Ref<Pipeline> m_UpscalePipeline;
		Ref<Pipeline> m_CombinePipeline;

		Ref<Framebuffer> m_Stages[16];

		Ref<RenderPass> m_PrefilterPass;
		Ref<RenderPass> m_DownscalePasses[16];
		Ref<RenderPass> m_UpscalePasses[16];
		Ref<RenderPass> m_CombinePass;
		
		Ref<UniformBuffer> m_PrefilterUB;
		Ref<UniformBuffer> m_UpscaleUB;
		Ref<UniformBuffer> m_CombineUB;

		uint32 m_Width = 0u;
		uint32 m_Height = 0u;
		uint32 m_MaxStages = 0u;
		uint32 m_Iterations = 0u;
		float m_UpsampleScale = 0.0f;

		BloomSettings m_Settings;
		Ref<Sampler> m_Sampler;
	};
}

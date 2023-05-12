#pragma once
#include "Core/Layer.h"
#include "Renderer/Framebuffer.h"
#include "Renderer/RenderPass.h"
#include "Renderer/DX11Buffers.h"
#include "Renderer/Pipeline.h"
#include "Renderer/Texture.h"
#include "Renderer/ParticleSystem.h"
#include "Renderer/Sampler.h"
#include "Renderer/BloomProcessor.h"

namespace DT
{
	class BloomLayer : public Layer
	{
	public:
		virtual void OnAttach() override;
		virtual void OnUpdate(float dt) override;
		virtual void OnEvent(Event& event) override;
		virtual void OnDetach() override;
		virtual void OnRender() override;
		virtual void OnUIRender() override;
		void InitBloom();
		void ExecuteBloom();
	private:
		float m_Time = 0.0f;
		glm::vec4 m_Color = { 1.0f, 0.0f, 0.0f, 1.0f };
		float m_Emission = 1.0f;

		Ref<Framebuffer> m_ScreenFramebuffer;  //represents the screen
		Ref<Framebuffer> m_GeoFramebuffer;     //represents the rendered geometry in HDR
		BloomProcessor m_BloomProcessor;
	};
}
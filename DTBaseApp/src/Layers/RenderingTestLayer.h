#pragma once
#include "Core/Layer.h"
#include "Renderer/Framebuffer.h"
#include "Renderer/RenderPass.h"
#include "Renderer/DX11Buffers.h"
#include "Renderer/Pipeline.h"
#include "Renderer/Texture.h"
#include "Renderer/ParticleSystem.h"

namespace DT
{
	class RenderingTestLayer : public Layer
	{
	public:
		virtual void OnAttach() override;
		virtual void OnUpdate(float dt) override;
		virtual void OnRender() override;
		virtual void OnUIRender() override;
		virtual void OnEvent(Event& event) override;
		virtual void OnDetach() override;
		float Animate(float speed = 1.0f)
		{
			return 0.5f + 0.5f * std::sin(2.0f * glm::pi<float>() * speed * m_Time);
		}
	private:
		Ref<Framebuffer> m_Framebuffer;
		Ref<RenderPass> m_RenderPass;
		Ref<Texture2D> m_Textures[2];
		float m_Time = 0.0f;
		float m_Thickness = 0.02f;
		float m_CircleThickness = 0.1f;
		float m_Angle = 0.0f;
		float m_Fade = 0.5f;
		float m_Radius = 0.2f;
		float m_Width = 0.05f;
		float m_Height = 0.05f;
		float m_Tiling = 1.0f;
		float m_Spacing = 0.230f;
		bool m_UseMouse = false;
		glm::vec2 m_Position = { 0.0f, 0.0f };
		glm::vec4 m_Color = { 1.0f, 1.0f, 1.0f, 1.0f };
		
		ParticleSystem m_ParticleSystem;
		ParticleProperties m_Properties;
	};
}
#pragma once
#include "Core/Layer.h"
#include "Renderer/Framebuffer.h"
#include "Renderer/RenderPass.h"
#include "Renderer/DX11Buffers.h"
#include "Renderer/Pipeline.h"
#include "Renderer/Texture.h"
#include "Renderer/ParticleSystem.h"
#include "Renderer/BloomProcessor.h"
#include "Audio/AudioEngine.h"

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
		void ResetParticles()
		{
			m_Properties.Position = { 0.0f, 0.0f };
			m_Properties.Velocity = { 0.0f, 0.0f };
			m_Properties.Acceleration = { 0.0f, 0.0f };
			m_Properties.VelocityVariation = 1.0f;
			m_Properties.Lifetime = 1.0f;
			m_Properties.RotationVariation = 10.0f;
			m_Properties.Friction = 0.0f;
			m_Properties.PositionVariation = 0.0f;
			m_Properties.StartSize = 0.05f;
			m_Properties.EndSize = 0.0f;
			m_Properties.StartEmission = 2.0f;
			m_Properties.EndEmission = 50.0f;
			m_Properties.StartColor = { 1.0f, 1.0f, 0.0f, 1.0f };
			m_Properties.EndColor = { 1.0f, 0.0f, 0.0f, 1.0f };
			m_RotationSpeed = 1.0f;
			m_VerticalRadius = 0.7f;
			m_HorizontalRadius = 0.7f;
			m_HorizontalSpeed = 1;
			m_VerticalSpeed = 1;
			m_AttractionPoint.Position = { 0.5f, 0.5f };
			m_AttractionPoint.Strenght = 0.0f;
		}

		float Animate(float speed = 1.0f)
		{
			return 0.5f + 0.5f * std::sin(2.0f * glm::pi<float>() * speed * m_Time);
		}
	private:
		Ref<Framebuffer> m_ScreenFramebuffer;
		Ref<Framebuffer> m_GeoFramebuffer;

		float m_Time = 0.0f;
		bool m_ImGuiEnabled = true;

		float m_CircleThickness = 1.0f;
		float m_Fade = 0.0f;
		float m_Width = 1.0f;
		float m_Height = 1.0f;

		float m_MasterVolume = 1.0f;

		float m_SoundVolume = 1.0f;
		float m_SoundPitch = 1.0f;
		float m_SoundPan = 0.0f;

		float m_MusicVolume = 1.0f;
		float m_MusicPitch = 1.0f;
		float m_MusicPan = 0.0f;

		float m_EffectsVolume = 1.0f;
		float m_EffectsPitch = 1.0f;
		float m_EffectsPan = 0.0f;

		bool m_UseMouse = false;

		bool m_RenderQuads = false;
		bool m_RenderCircles = true;
		
		bool m_EnableRotation = false;
		float m_RotationSpeed = 1.0f;
		float m_VerticalRadius = 0.7f;
		float m_HorizontalRadius = 0.7f;
		int32 m_HorizontalSpeed = 1;
		int32 m_VerticalSpeed = 1;

		BloomProcessor m_BloomProcessor;
		ParticleSystem m_ParticleSystem;
		ParticleProperties m_Properties;
		AttractionPoint m_AttractionPoint;

		SoundGroup m_MusicGroup;
		SoundGroup m_EffectsGroup;
		Ref<AudioNodes::LowPassFilter> m_LowPassFilter;
		std::vector<Ref<SoundEffect>> m_SoundEffects;
		std::vector<Ref<Sound>> m_Sounds;
		uint32 m_CurrentSound = 0u;
	};
}
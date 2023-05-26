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
			m_Properties.StartSize = 0.03f;
			m_Properties.EndSize = 0.0f;
			m_Properties.StartEmission = 10.0f;
			m_Properties.EndEmission = 50.0f;
			m_Properties.StartColor = { 1.0f, 0.0f, 1.0f, 1.0f };
			m_Properties.EndColor = { 0.0f, 1.0f, 1.0f, 1.0f };
			m_RotationSpeed = 1.0f;
			m_VerticalRadius = 0.7f;
			m_HorizontalRadius = 0.7f;
			m_HorizontalSpeed = 1;
			m_VerticalSpeed = 1;
			m_AttractionPoint.Position = { 0.5f, 0.5f };
			m_AttractionPoint.Strenght = 0.0f;
			m_EmissionRate = 60.0f;

			BloomSettings& settings = m_BloomProcessor.GetSettings();
			settings.Clamp = 0.0f;
			settings.Intensity = 0.05f;
			settings.Knee = 0.5f;
			settings.Radius = 6.5f;
			settings.Threshold = 0.8f;
		}

		void ResetAudio()
		{
			m_MasterVolume = 1.0f;

			m_MusicGroupVolume = 1.0f;
			m_MusicGroupPitch = 1.0f;
			m_MusicGroupPan = 0.0f;
			m_MusicGroup.SetVolume(m_MusicGroupVolume);
			m_MusicGroup.SetPitch(m_MusicGroupPitch);
			m_MusicGroup.SetPan(m_MusicGroupPan);

			m_EffectsGroupVolume = 1.0f;
			m_EffectsGroupPitch = 1.0f;
			m_EffectsGroupPan = 0.0f;
			m_EffectsGroup.SetVolume(m_EffectsGroupVolume);
			m_EffectsGroup.SetPitch(m_EffectsGroupPitch);
			m_EffectsGroup.SetPan(m_EffectsGroupPan);

			m_MusicVolume = 1.0f;
			m_MusicPitch = 1.0f;
			m_MusicPan = 0.0f;
			m_MusicFadeMilliseconds = 3000u;
			m_MusicFadeStartVolume = 0.0f;
			m_MusicFadeEndVolume = 1.0f;
			m_CursorChanged = false;
			
			for (auto& sound : m_Sounds)
			{
				sound->SetVolume(m_MusicVolume);
				sound->SetPitch(m_MusicPitch);
				sound->SetPan(m_MusicPan);
				sound->SetFade(m_MusicFadeMilliseconds, m_MusicFadeStartVolume, m_MusicFadeEndVolume);
			}
		}

		void ResetFilters()
		{
			m_LowPassFilter->UpdateParameters(20'000.0f);
			m_HighPassFilter->UpdateParameters(1.0f);
		}

		void ResetReverb()
		{
			m_ReverbNode->SetDamping(0.25f);
			m_ReverbNode->SetDryWet(0.0f);
			m_ReverbNode->SetRoomSize(0.5f);
			m_ReverbNode->SetStereoWidth(1.0f);
			m_ReverbNode->SetInputStereoWidth(0.0f);
			m_ReverbNode->SetMode(0.0f);
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
		float m_EmissionRate = 60.0f;

		float m_MasterVolume = 1.0f;

		float m_MusicGroupVolume = 1.0f;
		float m_MusicGroupPitch = 1.0f;
		float m_MusicGroupPan = 0.0f;

		float m_CursorPosition = 0.0f;
		float m_MusicVolume = 1.0f;
		float m_MusicPitch = 1.0f;
		float m_MusicPan = 0.0f;
		uint64 m_MusicFadeMilliseconds = 3000u;
		float m_MusicFadeStartVolume = 0.0f;
		float m_MusicFadeEndVolume = 1.0f;
		bool m_DelayEnabled = false;
		bool m_LinkMusicToParticles = false;
		bool m_CursorChanged = false;

		float m_EffectsGroupVolume = 1.0f;
		float m_EffectsGroupPitch = 1.0f;
		float m_EffectsGroupPan = 0.0f;

		bool m_UseMouse = false;

		bool m_RenderQuads = false;
		bool m_RenderCircles = true;
		bool m_Bounce = false;
		
		bool m_EnableRotation = false;
		float m_RotationSpeed = 1.0f;
		float m_VerticalRadius = 0.7f;
		float m_HorizontalRadius = 0.7f;
		int32 m_HorizontalSpeed = 1;
		int32 m_VerticalSpeed = 1;
		int32 m_SelectedOption = 2;

		BloomProcessor m_BloomProcessor;
		ParticleSystem m_ParticleSystem;
		ParticleProperties m_Properties;
		AttractionPoint m_AttractionPoint;

		Ref<AudioNodes::LowPassFilter> m_LowPassFilter;
		Ref<AudioNodes::HighPassFilter> m_HighPassFilter;
		Ref<AudioNodes::Delay> m_DelayNode;
		Ref<AudioNodes::Reverb> m_ReverbNode;
		SoundGroup m_MusicGroup;
		SoundGroup m_EffectsGroup;
		std::vector<Ref<SoundEffect>> m_SoundEffects;
		std::vector<Ref<Sound>> m_Sounds;
		uint32 m_CurrentSound = 0u;
	};
}
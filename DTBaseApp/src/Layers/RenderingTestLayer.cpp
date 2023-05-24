#include "RenderingTestLayer.h"
#include "Renderer/Renderer.h"
#include "Core/Application.h"
#include "Renderer/Renderer2D.h"
#include "imgui.h"
#include "Core/Input.h"

namespace DT
{
	void RenderingTestLayer::OnAttach()
	{
		Application::Get().GetWindow().SetSizeLimits(100,100,16'000,9'000);

		FramebufferSpecification speciFICAtion{};
		speciFICAtion.SwapchainTarget = true;
		m_ScreenFramebuffer = CreateRef<Framebuffer>(speciFICAtion);

		FramebufferSpecification geoFramebufferSpec{};
		geoFramebufferSpec.Format = ImageFormat::RGBA16F;
		geoFramebufferSpec.SwapchainTarget = false;
		m_GeoFramebuffer = Framebuffer::Create(geoFramebufferSpec);

		Renderer2D::SetTargetFramebuffer(m_GeoFramebuffer);

		ResetParticles();
		for (auto& dirEntry : std::filesystem::recursive_directory_iterator("assets/sounds"))
		{
			std::filesystem::path filePath = dirEntry.path();

			if (filePath.extension() == ".mp3")
				m_Sounds.emplace_back(Sound::Create(filePath.string().c_str(), &m_MusicGroup));
			else if(filePath.extension() == ".wav")
				m_SoundEffects.emplace_back(SoundEffect::Create(filePath.string().c_str(), &m_EffectsGroup));
		}

		m_LowPassFilter = AudioNodes::LowPassFilter::Create(20'000.0f);
		m_HighPassFilter = AudioNodes::HighPassFilter::Create(1.0f);
		m_DelayNode = AudioNodes::Delay::Create(0.25f, 0.4f);
		m_ReverbNode = AudioNodes::Reverb::Create();
		m_ReverbNode->SetDryWet(0.0f);

		Audio::Connect(m_MusicGroup.GetHandle(), m_LowPassFilter->GetHandle());
		Audio::Connect(m_EffectsGroup.GetHandle(), m_LowPassFilter->GetHandle());
		Audio::Connect(m_LowPassFilter->GetHandle(), m_ReverbNode->GetHandle());
		Audio::Connect(m_ReverbNode->GetHandle(), m_HighPassFilter->GetHandle());
		Audio::ConnectToEndpoint(m_HighPassFilter->GetHandle());

	}

	void RenderingTestLayer::OnUpdate(float dt)
	{
		m_Time = m_Time + dt;
		m_ParticleSystem.SetAttractionPoint(m_AttractionPoint.Position, m_AttractionPoint.Strenght);
		m_ParticleSystem.OnUpdate(dt);

		//Control particles with mouse 
		if (m_UseMouse)
		{
			if (Input::MouseIsPressed(Mouse::Left))
			{
				glm::vec2 mousePosition;
				float aspect = m_ScreenFramebuffer->GetAspectRatio();
				mousePosition.x = ((float)Input::GetMouseX() / m_ScreenFramebuffer->GetWidth() - 0.5f) * 2.0f * aspect;
				mousePosition.y = -((float)Input::GetMouseY() / m_ScreenFramebuffer->GetHeight() - 0.5f) * 2.0f;
				m_Properties.Position = mousePosition;
				m_ParticleSystem.EmitParticle(m_Properties);
			}
		}
		else
		{
			if (m_EnableRotation)
			{
				m_Properties.Position.x = m_HorizontalRadius * std::cos(m_RotationSpeed * m_HorizontalSpeed * m_Time);
				m_Properties.Position.y = m_VerticalRadius * std::sin(m_RotationSpeed * m_VerticalSpeed * m_Time);
			}
			else if(m_LinkMusicToParticles)
			{
				float frame = Audio::PeekOutput();
				float frameAbs = std::abs(frame);
				m_BloomProcessor.GetSettings().Intensity = 0.1f * frameAbs;
				m_Properties.Position.y = frame;
				m_Properties.Position.x = 1.5f;
				m_Properties.VelocityVariation = 0.0f;
				m_Properties.Velocity.x = -2.0f;
				m_Properties.StartColor.r = frameAbs;
				m_Properties.StartColor.g = 0.0f;
				m_Properties.StartColor.b = 1.0f - frameAbs;
			}

			m_ParticleSystem.EmitParticle(m_Properties);
		}
	}

	void RenderingTestLayer::OnRender()
	{
		m_GeoFramebuffer->ClearAttachment({ 0.0f, 0.0f, 0.0f, 1.0f });
		Renderer2D::BeginScene();

		m_ParticleSystem.OnRender([&](const ParticleSystem::Particle& particle)
		{
			glm::vec4 color = particle.CurrentColor;
			color.r = color.r * particle.CurrentEmission;
			color.g = color.g * particle.CurrentEmission;
			color.b = color.b * particle.CurrentEmission;

			if(m_RenderCircles)
				Renderer2D::DrawCircle(particle.Position, particle.CurrentSize, m_CircleThickness, m_Fade, color);

			if (m_RenderQuads)
				Renderer2D::DrawRotatedQuad(particle.Position, m_Width * particle.CurrentSize, m_Height * particle.CurrentSize, particle.Angle, color);
		});

		Renderer2D::DrawCircle(m_AttractionPoint.Position, 0.015f * std::abs(m_AttractionPoint.Strenght), 0.1f, 0.1f, { 0.1f, 0.1f, 0.1f, 1.0f });
		Renderer2D::EndScene();

		m_BloomProcessor.Execute(m_GeoFramebuffer->GetImage(), m_ScreenFramebuffer);
	}

	void RenderingTestLayer::OnUIRender()
	{
		if (m_ImGuiEnabled)
		{
			ImGui::ShowDemoWindow();
			ImGui::Begin("Particle System");

			if (ImGui::Checkbox("Use mouse", &m_UseMouse))
				m_Properties.Position = { 0.0f, 0.0f };

			ImGui::SameLine();
			if (ImGui::Button("Reset"))
				ResetParticles();

			ImGui::DragFloat2("Attractor Position", glm::value_ptr(m_AttractionPoint.Position), 0.01f);
			ImGui::SliderFloat("Attractor Strenght", &m_AttractionPoint.Strenght, -10.0f, 10.0f);
			ImGui::Separator();

			if (!m_UseMouse)
				ImGui::DragFloat2("Emit Position", glm::value_ptr(m_Properties.Position), 0.005f);

			ImGui::SliderFloat("Position Variation", &m_Properties.PositionVariation, 0.0f, 1.0f);
			ImGui::DragFloat2("Emit Velocity", glm::value_ptr(m_Properties.Velocity), 0.005f);
			ImGui::DragFloat2("Acceleration", glm::value_ptr(m_Properties.Acceleration), 0.005f);
			ImGui::SliderFloat("Friction", &m_Properties.Friction, 0.0f, 10.0f);
			ImGui::SliderFloat("Velocity Variation", &m_Properties.VelocityVariation, 0.0f, 2.0f);
			ImGui::SliderFloat("Rotation Variation", &m_Properties.RotationVariation, -10.0f, 10.0f);
			ImGui::Separator();
			ImGui::SliderFloat("Lifetime", &m_Properties.Lifetime, 0.0f, 10.0f);
			ImGui::Separator();
			ImGui::SliderFloat("Start Size", &m_Properties.StartSize, 0.0f, 0.2f);
			ImGui::SliderFloat("End Size", &m_Properties.EndSize, 0.0f, 0.2f);
			ImGui::Separator();
			ImGui::ColorEdit4("Start Color", glm::value_ptr(m_Properties.StartColor), ImGuiColorEditFlags_PickerHueWheel);
			ImGui::DragFloat("Start Emission", &m_Properties.StartEmission, 0.1f, 0.0f, 1000.0f);
			ImGui::ColorEdit4("End Color", glm::value_ptr(m_Properties.EndColor), ImGuiColorEditFlags_PickerHueWheel);
			ImGui::DragFloat("End Emission", &m_Properties.EndEmission, 0.1f, 0.0f, 1000.0f);
			ImGui::Separator();
			ImGui::Separator();
			ImGui::Checkbox("Render Quads", &m_RenderQuads);
			ImGui::SameLine();
			ImGui::Checkbox("Render Circles", &m_RenderCircles);

			if (m_RenderQuads)
			{
				ImGui::TextColored({ 1.0f, 0.5f, 0.3f, 1.0f }, "Quad Settings");
				ImGui::SliderFloat("Width", &m_Width, 0.0f, 2.0f);
				ImGui::SliderFloat("Height", &m_Height, 0.0f, 2.0f);
			};

			if (m_RenderCircles)
			{
				ImGui::TextColored({ 1.0f, 0.5f, 0.3f, 1.0f }, "Circle Settings");
				ImGui::SliderFloat("Fade", &m_Fade, 0.0f, 1.5f);
				ImGui::SliderFloat("Circle thickness", &m_CircleThickness, 0.0f, 1.0f);
			};

			ImGui::Separator();
			ImGui::Checkbox("Enable Rotation", &m_EnableRotation);

			if (m_EnableRotation)
			{
				ImGui::TextColored({ 1.0f, 0.5f, 0.3f, 1.0f }, "Rotation settings");
				ImGui::SliderFloat("Horizontal Radius", &m_HorizontalRadius, 0.0f, 1.5f);
				ImGui::SliderFloat("Vertical Radius", &m_VerticalRadius, 0.0f, 1.5f);
				ImGui::SliderFloat("Rotation speed", &m_RotationSpeed, 0.0f, 5.0f);
				ImGui::SliderInt("Horizontal Speed", &m_HorizontalSpeed, -5, 5);
				ImGui::SliderInt("Vertical Speed", &m_VerticalSpeed, -5, 5);
			}

			ImGui::Separator();
			ImGui::Separator();
			ImGui::Text("DrawCalls: %d", Renderer2D::GetStatistics().DrawCalls);
			ImGui::Text("Polygon Count: %d", Renderer2D::GetStatistics().PolygonCount);
			ImGui::Text("FPS: %f", ImGui::GetIO().Framerate);
			ImGui::End();

			BloomSettings& settings = m_BloomProcessor.GetSettings();

			ImGui::Begin("Bloom");
			ImGui::SliderFloat("Intensity", &settings.Intensity, 0.0f, 0.1f);
			ImGui::SliderFloat("Radius", &settings.Radius, 0.0f, 10.0f);
			ImGui::SliderFloat("Threshold", &settings.Threshold, 0.0f, 10.0f);
			ImGui::SliderFloat("Knee", &settings.Knee, 0.0f, 1.0f);
			ImGui::SliderFloat("Clamp", &settings.Clamp, 0.0f, 1000.0f);
			ImGui::End();

			ImGui::Begin("Music");

			if (ImGui::SliderFloat("Master Volume", &m_MasterVolume, 0.0f, 1.0f))
				Audio::SetMasterVolume(m_MasterVolume);

			ImGui::SeparatorText("Music Group");

			if (ImGui::Button("Play##Music"))
				m_MusicGroup.Play();

			ImGui::SameLine();

			if (ImGui::Button("Pause##Music"))
				m_MusicGroup.Pause();

			if (ImGui::SliderFloat("Volume##Music", &m_MusicVolume, 0.0f, 1.0f))
				m_MusicGroup.SetVolume(m_MusicVolume);

			if (ImGui::SliderFloat("Pitch##Music", &m_MusicPitch, 0.5f, 1.5f))
				m_MusicGroup.SetPitch(m_MusicPitch);

			if (ImGui::SliderFloat("Pan##Music", &m_MusicPan, -1.0f, 1.0f))
				m_MusicGroup.SetPan(m_MusicPan);

			ImGui::Separator();

			{
				float value = m_LowPassFilter->GetFrequency();
				if (ImGui::SliderFloat("LPF Frequency", &value, 1.0f, 20'000.0f,"%.3f (Hz)", ImGuiSliderFlags_Logarithmic))
					m_LowPassFilter->UpdateParameters(value);
			}

			{
				float value = m_HighPassFilter->GetFrequency();
				if (ImGui::SliderFloat("HPF Frequency", &value, 1.0f, 20'000.0f,"%.3f (Hz)", ImGuiSliderFlags_Logarithmic))
					m_HighPassFilter->UpdateParameters(value);
			}

			ImGui::SeparatorText("Reverb");

			{
				float value = m_ReverbNode->GetDryWet();
				if (ImGui::SliderFloat("Dry/Wet", &value, 0.0f, 1.0f))
					m_ReverbNode->SetDryWet(value);
			}

			{
				float value = m_ReverbNode->GetRoomSize();
				if (ImGui::SliderFloat("Room Size", &value, 0.0f, 1.0f))
					m_ReverbNode->SetRoomSize(value);
			}

			{
				float value = m_ReverbNode->GetDamping();
				if (ImGui::SliderFloat("Damping", &value, 0.0f, 1.0f))
					m_ReverbNode->SetDamping(value);
			}

			{
				float value = m_ReverbNode->GetStereoWidth();
				if (ImGui::SliderFloat("Stereo Width", &value, 0.0f, 1.0f))
					m_ReverbNode->SetStereoWidth(value);
			}

			{
				float value = m_ReverbNode->GetInputStereoWidth();
				if (ImGui::SliderFloat("Input Stereo Width", &value, 0.0f, 1.0f))
					m_ReverbNode->SetInputStereoWidth(value);
			}

			{
				float value = m_ReverbNode->GetMode();
				if (ImGui::SliderFloat("Mode", &value, 0.0f, 1.0f))
					m_ReverbNode->SetMode(value);
			}

			ImGui::Separator();

			if (ImGui::BeginCombo("Music", m_Sounds[m_CurrentSound]->GetName().c_str()))
			{
				for (uint64 i = 0u; i < m_Sounds.size(); i++)
				{
					bool isSelected = (m_CurrentSound == i);
					if (ImGui::Selectable(m_Sounds[i]->GetName().c_str(), isSelected))
					{
						if(m_CurrentSound != i)
							m_Sounds[m_CurrentSound]->Stop();

						m_CurrentSound = (uint32)i;
					}

					if (isSelected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}

			if (ImGui::Button("Play"))
				m_Sounds[m_CurrentSound]->Play();

			ImGui::SameLine();

			if (ImGui::Button("Pause"))
				m_Sounds[m_CurrentSound]->Pause();

			ImGui::SameLine();

			if (ImGui::Button("Stop"))
				m_Sounds[m_CurrentSound]->Stop();

			float soundLength = m_Sounds[m_CurrentSound]->GetLengthInSeconds();
			float cursorPosition = m_Sounds[m_CurrentSound]->GetCursorInSeconds();
			uint32 minutes = (uint32)cursorPosition / 60u;
			uint32 seconds = (uint32)cursorPosition % 60u;
			std::string formattedCursor = std::format("{}m:{}s", minutes, seconds);
			if (ImGui::SliderFloat("##CursorPosition", &cursorPosition, 0.0f, soundLength, formattedCursor.c_str()))
				m_Sounds[m_CurrentSound]->SetCursorInSeconds(cursorPosition);

			if (ImGui::SliderFloat("Volume", &m_SoundVolume, 0.0f, 1.0f))
				m_Sounds[m_CurrentSound]->SetVolume(m_SoundVolume);

			if (ImGui::SliderFloat("Pitch", &m_SoundPitch, 0.5f, 1.5f))
				m_Sounds[m_CurrentSound]->SetPitch(m_SoundPitch);

			if (ImGui::SliderFloat("Pan", &m_SoundPan, -1.0f, 1.0f))
				m_Sounds[m_CurrentSound]->SetPan(m_SoundPan);

			uint64 min = 0u;
			uint64 max = 10'000u;
			if (ImGui::SliderScalar("Fade Duration", ImGuiDataType_U64, &m_MusicFadeMilliseconds, &min, &max, "%d (ms)"))
				m_Sounds[m_CurrentSound]->SetFade(m_MusicFadeMilliseconds, m_MusicFadeStartVolume, m_MusicFadeEndVolume);

			if (ImGui::SliderFloat("Fade Start Volume", &m_MusicFadeStartVolume, 0.0f, 1.0f))
				m_Sounds[m_CurrentSound]->SetFade(m_MusicFadeMilliseconds, m_MusicFadeStartVolume, m_MusicFadeEndVolume);

			if (ImGui::SliderFloat("Fade End Volume", &m_MusicFadeEndVolume, 0.0f, 1.0f))
				m_Sounds[m_CurrentSound]->SetFade(m_MusicFadeMilliseconds, m_MusicFadeStartVolume, m_MusicFadeEndVolume);

			ImGui::Checkbox("Link Music To Particles", &m_LinkMusicToParticles);
			ImGui::SameLine();

			if (ImGui::Checkbox("Enable Delay", &m_DelayEnabled))
			{
				if (m_DelayEnabled)
				{
					Audio::Connect(m_MusicGroup.GetHandle(), m_DelayNode->GetHandle());
					Audio::Connect(m_EffectsGroup.GetHandle(), m_DelayNode->GetHandle());
					Audio::Connect(m_DelayNode->GetHandle(), m_LowPassFilter->GetHandle());
					Audio::Connect(m_LowPassFilter->GetHandle(), m_ReverbNode->GetHandle());
					Audio::Connect(m_ReverbNode->GetHandle(), m_HighPassFilter->GetHandle());
				}
				else 
				{
					Audio::Connect(m_MusicGroup.GetHandle(), m_LowPassFilter->GetHandle());
					Audio::Connect(m_EffectsGroup.GetHandle(), m_LowPassFilter->GetHandle());
					Audio::Connect(m_LowPassFilter->GetHandle(), m_ReverbNode->GetHandle());
					Audio::Connect(m_ReverbNode->GetHandle(), m_HighPassFilter->GetHandle());
				}
			}

			ImGui::SeparatorText("Effects Group");

			if (ImGui::Button("Play##Effects"))
				m_EffectsGroup.Play();

			ImGui::SameLine();

			if (ImGui::Button("Pause##Effects"))
				m_EffectsGroup.Pause();

			if (ImGui::SliderFloat("Volume##Effects", &m_EffectsVolume, 0.0f, 1.0f))
				m_EffectsGroup.SetVolume(m_EffectsVolume);

			if (ImGui::SliderFloat("Pitch##Effects", &m_EffectsPitch, 0.5f, 1.5f))
				m_EffectsGroup.SetPitch(m_EffectsPitch);

			if (ImGui::SliderFloat("Pan##Effects", &m_EffectsPan, -1.0f, 1.0f))
				m_EffectsGroup.SetPan(m_EffectsPan);

			for (uint64 i = 0u; i < m_SoundEffects.size(); i++)
			{
				ImGui::PushID((int32)i);
				ImGui::SeparatorText(m_SoundEffects[i]->GetName().c_str());

				if (ImGui::Button("Play"))
				{
					SoundProperties properties{};
					properties.Volume = rand() / (float)RAND_MAX;
					properties.Pitch = 1.0f + 0.5f * rand() / (float)RAND_MAX - 0.25f;
					properties.Pan = rand() / (float)RAND_MAX - 0.5f;
					Audio::PlaySoundEffect(m_SoundEffects[i], properties);
				}
				ImGui::PopID();
			}
			ImGui::End();
		}
	}

	void RenderingTestLayer::OnEvent(Event& event)
	{
		Event::Dispatcher dispatcher(event);
		dispatcher.Dispatch<KeyPressedEvent>([&](KeyPressedEvent& key)
		{
			switch (key.GetKeyCode())
			{
				case Key::I:
				{
					m_ImGuiEnabled = !m_ImGuiEnabled;
					break;
				}
			}
			return false;
		});
	}

	void RenderingTestLayer::OnDetach()
	{
	}
}
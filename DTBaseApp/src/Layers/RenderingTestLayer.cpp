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

		//m_Sound = Sound::Create("assets/sounds/Tomb Raider III/ROOFS_00093.wav");
		m_Sound = Sound::Create("assets/sounds/Infected Mushroom & Ganja White Nights - Kill to Feel.mp3");
	}

	void RenderingTestLayer::OnUpdate(float dt)
	{
		m_Time = m_Time + dt;
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

		Renderer2D::EndScene();

		m_BloomProcessor.Execute(m_GeoFramebuffer->GetImage(), m_ScreenFramebuffer);
	}

	void RenderingTestLayer::OnUIRender()
	{
		if (m_ImGuiEnabled)
		{
			ImGui::Begin("Particle System");

			if (ImGui::Checkbox("Use mouse", &m_UseMouse))
				m_Properties.Position = { 0.0f, 0.0f };

			ImGui::SameLine();
			if (ImGui::Button("Reset"))
				ResetParticles();

			if (!m_UseMouse)
				ImGui::DragFloat2("Emit Position", glm::value_ptr(m_Properties.Position), 0.005f);

			ImGui::DragFloat2("Emit Velocity", glm::value_ptr(m_Properties.Velocity), 0.005f);
			ImGui::DragFloat2("Acceleration", glm::value_ptr(m_Properties.Acceleration), 0.005f);
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
				AudioEngine::SetMasterVolume(m_MasterVolume);

			ImGui::SeparatorText("Sounds");
			
			if (ImGui::Button("Play"))
				m_Sound->Play();

			ImGui::SameLine();

			if (ImGui::Button("Pause"))
				m_Sound->Pause();

			ImGui::SameLine();

			if (ImGui::Button("Stop"))
				m_Sound->Stop();

			if (ImGui::SliderFloat("Volume", &m_SoundVolume, 0.0f, 1.0f))
				m_Sound->SetVolume(m_SoundVolume);

			if (ImGui::SliderFloat("Pitch", &m_SoundPitch, 0.5f, 1.5f))
				m_Sound->SetPitch(m_SoundPitch);

			if (ImGui::SliderFloat("Pan", &m_SoundPan, -1.0f, 1.0f))
				m_Sound->SetPan(m_SoundPan);

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
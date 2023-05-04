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
		m_Framebuffer = CreateRef<Framebuffer>(speciFICAtion);

		RenderPassSpecification renderPassSpecification{};
		renderPassSpecification.ClearColor = { Animate(1.0f), 0.1f, 0.4f, 1.0f};
		renderPassSpecification.TargetFrameBuffer = m_Framebuffer;
		m_RenderPass = CreateRef<RenderPass>(renderPassSpecification);

		//m_Textures[0] = CreateRef<Texture2D>("assets/textures/M_FloorTiles1_Inst_0_BaseColor.png");
		//m_Textures[1] = CreateRef<Texture2D>("assets/textures/PBRPack/pbr14/albedo.png");
	}

	void RenderingTestLayer::OnUpdate(float dt)
	{
		m_Time = m_Time + dt;
		m_RenderPass->GetSpecification().ClearColor = { 0.15f * Animate(0.5f), 0.0f, 0.15f * Animate(0.3f), 1.0f};
		m_ParticleSystem.OnUpdate(dt);

		//Control particles with mouse 
		if (m_UseMouse)
		{
			if (Input::MouseIsPressed(Mouse::Left))
			{
				glm::vec2 mousePosition;
				mousePosition.x =((float)Input::GetMouseX() / Application::Get().GetWindow().GetWidth() - 0.5f) * 2.0f * 16.0f / 9.0f;
				mousePosition.y = - ((float)Input::GetMouseY() / Application::Get().GetWindow().GetHeight() - 0.5f) * 2.0f;
				m_Properties.Position = mousePosition;
				m_ParticleSystem.EmitParticle(m_Properties);
			}
		}
		else
		{
			m_ParticleSystem.EmitParticle(m_Properties);
		}
	}

	void RenderingTestLayer::OnRender()
	{
		Renderer::BeginRenderPass(m_RenderPass);

		Renderer2D::BeginScene();

		//Renderer2D::DrawQuad({ 0.5f, 0.5f }, 0.2f, 0.2f, { 0.0f, 0.4f, 0.9f, 1.0f });
		//Renderer2D::DrawQuad({ -0.5f, 0.5f }, 0.2f, 0.5f * Animate(0.2f), {0.0f, 0.4f, 0.9f, 1.0f});
		//Renderer2D::DrawQuad({ 0.0f, -0.5f }, 1.0f, 0.2f, { 0.0f, 0.4f, 0.9f, 1.0f });
		//
		//Renderer2D::DrawRotatedQuad({ 0.0f,0.0f}, 1.5f, 0.2f, glm::radians(m_Time * 7.0f), {1.0f, 0.5f,0.4f, 0.5f});
		//Renderer2D::DrawRotatedQuad({ 0.0f,0.0f}, 1.5f, 0.2f, glm::radians(-m_Time * 7.0f), {1.0f, 0.5f,0.4f, 0.5f});
		//
		//Renderer2D::DrawCircle({ 0.0f,0.0f }, 0.1f, Animate(0.2f), 0.05f, {Animate(1.0f), 0.8f, 0.9f, 0.5f});
		//Renderer2D::DrawCircle({ 0.5f,0.7f }, 0.1f, Animate(0.2f), 0.05f, {Animate(1.0f), 0.8f, 0.9f, 0.5f});
		//Renderer2D::DrawCircle({ -0.7f,0.2f }, 0.1f, Animate(0.2f), 0.05f, {Animate(1.0f), 0.8f, 0.9f, 0.5f});
		//
		//Renderer2D::DrawLine({ 0.0f,0.0f }, { 0.5f, 0.7f }, m_Time * 0.001f, { 1.0f, 1.0f, 0.0f, 0.7f });
		//Renderer2D::DrawLine({ 0.5f,0.7f }, { -0.7f, 0.2f }, m_Time * 0.001f, { 1.0f, 1.0f, 0.0f, 0.7f });
		//Renderer2D::DrawLine({ -0.7f,0.2f }, { 0.0f, 0.0f }, m_Time * 0.001f, { 1.0f, 1.0f, 0.0f, 0.7f });
		
		//Renderer2D::DrawRect({0.0f, 0.0f}, m_Width, m_Height, m_Thickness, { 1.0f, 1.0f, 0.8f, 1.0f });
		//Renderer2D::DrawRotatedQuad({ 1.0f, 0.5f }, 0.5f, 0.3f, m_Angle, { 1.0f, 0.7f, 0.0f, 1.0f });
		//Renderer2D::DrawRotatedRect({ 1.0f, 0.5f }, 0.5f, 0.3f, m_Thickness, m_Angle, { 1.0f, 0.1f, 0.2f, 1.0f });
		//Renderer2D::DrawCircle(m_Position, m_Radius, m_CircleThickness, m_Fade, m_Color);
		//Renderer2D::DrawTexturedQuad({ 0.0f, 0.0f }, m_Width, m_Height, m_Textures[0], m_Tiling, m_Color);
		//Renderer2D::DrawRotatedTexQuad({ 0.5f,0.5f }, m_Width, m_Height, m_Textures[1], m_Tiling, Animate(1.0f), m_Color);

		//for (uint32 i = 0u; i < 50u; i++)
		//{
		//	for (uint32 j = 0u; j < 50u; j++)
		//	{
		//		glm::vec2 position;
		//		position.x = -0.8f + i * m_Spacing;
		//		position.y = -0.8f + j * m_Spacing;
		//		Renderer2D::DrawTexturedQuad(position, m_Width, m_Height, m_Textures[(i + j) % 2u], m_Tiling, m_Color);
		//	}
		//}

		m_ParticleSystem.OnRender(m_Fade);

		Renderer2D::EndScene();

		Renderer::EndRenderPass();
	}

	void RenderingTestLayer::OnUIRender()
	{
		if (m_ImGuiEnabled)
		{
			ImGui::Begin("Particle System");
			ImGui::Checkbox("Use mouse", &m_UseMouse);

			if (!m_UseMouse)
				ImGui::DragFloat2("Emit Position", glm::value_ptr(m_Properties.Position), 0.005f);

			ImGui::DragFloat2("Emit Velocity", glm::value_ptr(m_Properties.Velocity), 0.005f);
			ImGui::SliderFloat("Velocity Variation", &m_Properties.VelocityVariation, 0.0f, 2.0f);
			ImGui::SliderFloat("Ang. Velocity Variation", &m_Properties.RotationVariation, -10.0f, 10.0f);
			ImGui::Separator();
			ImGui::SliderFloat("Lifetime", &m_Properties.Lifetime, 0.0f, 10.0f);
			ImGui::Separator();
			ImGui::SliderFloat("Fade", &m_Fade, 0.0f, 1.5f);
			ImGui::ColorEdit4("Start Color", glm::value_ptr(m_Properties.StartColor), ImGuiColorEditFlags_PickerHueWheel);
			ImGui::ColorEdit4("End Color", glm::value_ptr(m_Properties.EndColor), ImGuiColorEditFlags_PickerHueWheel);
			ImGui::SliderFloat("Start Size", &m_Properties.StartSize, 0.0f, 0.2f);
			ImGui::SliderFloat("End Size", &m_Properties.EndSize, 0.0f, 0.2f);
			ImGui::End();

			ImGui::Begin("test");
			//ImGui::ColorButton("Colore di merda di imgui", { 0.2f, 0.3f, 0.5f, 0.7f });
			//ImGui::Separator();
			//ImGui::SliderFloat("Thickness", &m_Thickness, 0.0f, 0.1f);
			//ImGui::SliderAngle("Angle", &m_Angle, 0.0f, 180.0f);
			//ImGui::SliderFloat("Width", &m_Width, 0.0f, 1.5f);
			//ImGui::SliderFloat("Height", &m_Height, 0.0f, 1.5f);
			//ImGui::Separator();
			//ImGui::SliderFloat("Circle thickness", &m_CircleThickness, 0.0f, 1.0f);
			//ImGui::SliderFloat2("Position", glm::value_ptr(m_Position), -1.0f, 1.0f);
			//ImGui::SliderFloat("Radius", &m_Radius, 0.0f, 1.0f);
			//ImGui::Separator();
			//ImGui::ColorEdit4("Color", glm::value_ptr(m_Color), ImGuiColorEditFlags_PickerHueWheel);
			//ImGui::Separator();
			//ImGui::SliderFloat("Tiling", &m_Tiling, 0.0f, 5.0f);
			//ImGui::SliderFloat("Spacing", &m_Spacing, 0.0f, 0.5f);
			ImGui::Text("DrawCalls: %d", Renderer2D::GetStatistics().DrawCalls);
			ImGui::Text("Polygon Count: %d", Renderer2D::GetStatistics().PolygonCount);
			ImGui::Text("FPS: %f", ImGui::GetIO().Framerate);
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
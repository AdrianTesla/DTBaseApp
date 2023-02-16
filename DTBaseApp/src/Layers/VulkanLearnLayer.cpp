#include "VulkanLearnLayer.h"
#include "Core/Application.h"
#include "Core/Input.h"
#include "Platform/PlatformUtils.h"

namespace DT
{
	void VulkanLearnLayer::OnAttach()
	{
		Application::Get().GetWindow().SetResizable(false);
	}

	void VulkanLearnLayer::OnUpdate(float dt)
	{
		m_TimeSteps[m_TimeStepIndex++] = dt;

		if (m_TimeStepIndex >= m_TimeSteps.size())
		{
			float fps = 0.0f;
			for (size_t i = 0u; i < m_TimeSteps.size(); i++)
				fps += m_TimeSteps[i];
			fps = (float)m_TimeSteps.size() / fps;
			Application::Get().GetWindow().SetTitle(std::format("Dodge this! {} FPS", std::ceilf(fps)));
			m_TimeStepIndex = 0u;
		}
	}

	void VulkanLearnLayer::OnEvent(Event& event)
	{
		Event::Dispatcher dispatcher(event);
		dispatcher.Dispatch<KeyPressedEvent>([](KeyPressedEvent& key)
		{
			switch (key.GetKeyCode())
			{
				case Key::A:
					break;
				case Key::Q:
					Application::Get().CloseApplication();
					break;
				case Key::G:
					break;
				case Key::C:
					break;
				case Key::F:
					Application::Get().Run();
					break;
				case Key::Enter:
					static float opacity = 1.0f;
					opacity = (opacity == 1.0f ? 0.5f : 1.0f);
					if (Input::KeyIsPressed(Key::LeftAlt))
						Application::Get().GetWindow().SetOpacity(opacity);
					break;
			}
			return false;
		});

		dispatcher.Dispatch<MouseButtonPressedEvent>([&](MouseButtonPressedEvent& button)
		{
			switch(button.GetButtonCode())
			{
				case Mouse::Left:
					break;
				case Mouse::Middle:
					break;
				case Mouse::Right:
					break;
			}
			return false;
		});
	}

	void VulkanLearnLayer::OnDetach()
	{
	}
}
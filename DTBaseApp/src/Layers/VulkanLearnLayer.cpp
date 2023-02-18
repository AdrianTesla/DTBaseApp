#include "VulkanLearnLayer.h"
#include "Core/Application.h"
#include "Core/Input.h"
#include "Platform/PlatformUtils.h"

namespace DT
{
	void VulkanLearnLayer::OnAttach()
	{
		//Application::Get().GetWindow().SetFixedAspectRatio(4, 3);
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
				case Key::M:
					Application::Get().GetWindow().Minimize();
					break;
				case Key::C:
					break;
				case Key::Enter:
					static bool windowed = false;
					windowed = !windowed;
					if (Input::KeyIsPressed(Key::LeftAlt))
					{
						if (windowed)
							Application::Get().GetWindow().ToFullscreen();
						else
							Application::Get().GetWindow().ToWindowed();
					}
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
#include "VulkanLearnLayer.h"
#include "Core/Application.h"
#include "Core/Input.h"
#include "Platform/PlatformUtils.h"

namespace DT
{
	void VulkanLearnLayer::OnAttach()
	{
	}

	void VulkanLearnLayer::OnUpdate(float dt)
	{
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
				case Key::G:
					break;
				case Key::C:
					break;
				case Key::F:
					Application::Get().GetWindow().ToFullscreen();
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
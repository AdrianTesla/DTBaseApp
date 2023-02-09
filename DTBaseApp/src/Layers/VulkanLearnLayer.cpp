#include "VulkanLearnLayer.h"
#include "Core/Application.h"
#include "Core/Input.h"

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
				{
					break;
				}
				case Key::B:
				{
					break;
				}
				case Key::C:
				{
					break;
				}
			}
			return false;
		});
	}

	void VulkanLearnLayer::OnDetach()
	{
	}
}
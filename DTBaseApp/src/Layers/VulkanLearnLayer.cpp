#include "VulkanLearnLayer.h"
#include "Core/Application.h"

namespace DT
{
	void VulkanLearnLayer::OnAttach()
	{
		LOG_TRACE("Attached!");

	}

	void VulkanLearnLayer::OnUpdate(float dt)
	{
	}

	void VulkanLearnLayer::OnEvent(Event& event)
	{
		Event::Dispatcher dispatcher(event);
		dispatcher.Dispatch<KeyPressedEvent>([&](KeyPressedEvent& key)
		{
			if (key.GetRepeatCount() > 0)
				return false;

			switch (key.GetKeyCode())
			{
				case Key::M:
				{
					Application::Get().GetWindow().Maximize();
					break;
				}
				case Key::F:
				{
					static bool windowed = true;
					if (windowed)
						Application::Get().GetWindow().ToFullscreen();
					else
						Application::Get().GetWindow().ToWindowed();
					windowed = !windowed;
					break;
				}
				case Key::A:
				{
					static bool fixed = false;
					fixed = !fixed;
					if (fixed)
						Application::Get().GetWindow().SetFixedAspectRatio(16, 9);
					else
						Application::Get().GetWindow().SetFixedAspectRatio(-1, -1);
					break;
				}
				case Key::C:
				{
					Application::Get().GetWindow().SetMousePosition(200, 200);
					break;
				}
				case Key::U:
				{
					LOG_TRACE(Application::Get().GetWindow().GetClipboardString());
					break;
				}
				case Key::Up:
				{
					m_Opacity += 0.1f;
					m_Opacity = std::clamp(m_Opacity, 0.0f, 1.0f);
					Application::Get().GetWindow().SetOpacity(m_Opacity);
					break;
				}
				case Key::Down:
				{
					m_Opacity -= 0.1f;
					m_Opacity = std::clamp(m_Opacity, 0.0f, 1.0f);
					Application::Get().GetWindow().SetOpacity(m_Opacity);
					break;
				}
				case Key::T:
				{
					Application::Get().GetWindow().SetTitle("New title!");
					break;
				}
				case Key::D:
				{
					static bool decorated = true;
					decorated = !decorated;
					Application::Get().GetWindow().SetDecorated(decorated);
					break;
				}
				case Key::R:
				{
					static bool resizable = true;
					resizable = !resizable;
					Application::Get().GetWindow().SetResizable(resizable);
					break;
				}
				case Key::S:
				{
					Application::Get().GetWindow().CenterWindow();
					break;
				}
				case Key::N:
				{
					Application::Get().GetWindow().SetSizeLimits(200, 200, 500, 300);
					break;
				}
			}
			return false;
		});
		dispatcher.Dispatch<WindowFocusEvent>([](WindowFocusEvent& e) 
		{
			LOG_TRACE("Window focused? {}", e.IsFocused());
			return false;
		});
	}

	void VulkanLearnLayer::OnDetach()
	{
		LOG_TRACE("Detached!");
	}
}
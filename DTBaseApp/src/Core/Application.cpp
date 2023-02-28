#include "Application.h"
#include "Layers/VulkanLearnLayer.h"
#include "Input.h"
#include <GLFW/glfw3.h>

namespace DT
{
	Application::Application(const ApplicationSpecification& specification)
		: m_Specification(specification)
	{
		s_Instance = this;

		m_Window = Window::Create(m_Specification.Window);
		m_Window->SetEventCallBack(BIND_FUNC(OnEvent));

		m_RendererContext = RendererContext::Create(m_Window);
		m_RendererContext->Init();

		Renderer::Init();

		PushLayer(new VulkanLearnLayer);
	}

	Application::~Application()
	{
		for (Layer* layer : m_Layers) {
			layer->OnDetach();
			delete layer;
		}

		Renderer::Shutdown();
	}

	void Application::PushLayer(Layer* layer)
	{
		m_Layers.emplace_back(layer);
		layer->OnAttach();
	}

	void Application::Run()
	{
		Timer timer;
		while (m_AppRunning) 
		{
			m_Window->ProcessEvents();
			if (!m_AppMinimized) 
			{
				UpdatePhase(timer.Mark());
				RenderPhase();
				m_FrameCount++;
			}
		}
	}

	void Application::UpdatePhase(float dt)
	{
		m_TimeSteps[m_TimeStepIndex++] = dt;
		if (m_TimeStepIndex >= m_TimeSteps.size())
		{
			float fps = 0.0f;
			for (size_t i = 0u; i < m_TimeSteps.size(); i++)
				fps += m_TimeSteps[i];
			fps = std::ceilf((float)m_TimeSteps.size() / fps);
			m_Window->SetTitle(std::format("Dodge This! Vulkan {} FPS", fps));
			m_TimeStepIndex = 0u;
		}

		for (Layer* layer : m_Layers)
			layer->OnUpdate(dt);
	}

	void Application::RenderPhase()
	{
		Renderer::BeginFrame();

		for (Layer* layer : m_Layers)
			layer->OnRender();

		Renderer::EndFrame();
	}

	void Application::OnEvent(Event& event)
	{
		for (size_t i = m_Layers.size(); i > 0u; i--)
			m_Layers[i - 1]->OnEvent(event);

		Event::Dispatcher dispatcher(event);
		dispatcher.Dispatch<WindowClosedEvent>([&](WindowClosedEvent& e)
		{
			m_AppRunning = false;
			return false;
		});
		dispatcher.Dispatch<WindowIconifiedEvent>([&](WindowIconifiedEvent& e)
		{
			m_AppMinimized = e.Minimized();
			LOG_TRACE(e.Minimized() ? "App minimized" : "App restored");
			return false;
		});
		dispatcher.Dispatch<WindowResizeEvent>([&](WindowResizeEvent& e)
		{
			m_AppMinimized = e.IsDegenerate();
			m_RendererContext->OnWindowResize();
			return false;
		});
		dispatcher.Dispatch<KeyPressedEvent>([](KeyPressedEvent& key)
		{
			switch (key.GetKeyCode())
			{
				case Key::Enter:
				{
					static bool windowed = false;
					if (Input::KeyIsPressed(Key::LeftAlt))
					{
						windowed = !windowed;
						if (windowed)
							Application::Get().GetWindow().ToFullscreen();
						else
							Application::Get().GetWindow().ToWindowed();
					}
				} break;
				case Key::PadAdd:
				{
					if (!key.GetRepeatCount()) {
						static bool vsync = false;
						Renderer::SetVerticalSync(vsync);
						vsync = !vsync;
					}
				} break;
			}
			return false;
		});
	}

	float Application::GetTime() const
	{
		return (float)glfwGetTime();
	}
}
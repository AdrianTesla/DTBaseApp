#include "Application.h"
#include "Layers/VulkanLearnLayer.h"

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

		PushLayer(new VulkanLearnLayer);
	}

	Application::~Application()
	{
		for (Layer* layer : m_Layers)
		{
			layer->OnDetach();
			delete layer;
		}
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
		for (Layer* layer : m_Layers)
			layer->OnUpdate(dt);
	}

	void Application::RenderPhase()
	{
		for (Layer* layer : m_Layers)
			layer->OnRender();

		m_RendererContext->DrawFrameTest();
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
	}
}
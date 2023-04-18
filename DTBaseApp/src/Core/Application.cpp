#include "Application.h"
#include "Layers/TestLayer.h"


namespace DT
{
	Application::Application(const ApplicationSpecification& specification)
		: m_Specification(specification)
	{
		s_Instance = this;

		m_Window = Window::Create(m_Specification.WindowSpecification);
		m_Window->SetEventCallBack(BIND_FUNC(OnEvent));
		m_GraphicsContext = new GraphicsContext(m_Window);
		m_GraphicsContext->Init();

		if (std::filesystem::exists(m_Specification.WorkingDirectory))
			std::filesystem::current_path(m_Specification.WorkingDirectory);

		LOG_INFO("Working Directory: {}", std::filesystem::current_path().string());

		PushLayer(new TestLayer);
	}

	Application::~Application()
	{
		for (Layer* layer : m_Layers)
		{
			layer->OnDetach();
			delete layer;
		}

		delete m_GraphicsContext;
		delete m_Window;
	}

	void Application::PushLayer(Layer* layer)
	{
		layer->OnAttach();
		m_Layers.emplace_back(layer);
	}

	void Application::Run()
	{
		Timer timer;
		while (m_AppRunning)
		{
			m_Window->ProcessEvents();
			UpdatePhase(timer.Mark());
			RenderPhase();
		}
	}

	void Application::UpdatePhase(float dt)
	{
		for (Layer* layer : m_Layers)
			layer->OnUpdate(dt);
	}

	void Application::RenderPhase()
	{
		m_GraphicsContext->BeginFrame();

		for (Layer* layer : m_Layers)
			layer->OnRender(); 

		m_GraphicsContext->Present();
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
	}
}
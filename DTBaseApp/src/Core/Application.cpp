#include "Application.h"
#include "Layers/TestLayer.h"
#include "Renderer/Renderer.h"
#include "Layers/RenderingTestLayer.h"
#include "Layers/BloomLayer.h"
#include "Audio/AudioEngine.h"


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

		Renderer::Init();
		Audio::Init();
		
		if (std::filesystem::exists(m_Specification.WorkingDirectory))
			std::filesystem::current_path(m_Specification.WorkingDirectory);

		LOG_INFO("Working Directory: {}", std::filesystem::current_path().string());

		PushLayer(new BloomLayer);

		if (m_Specification.EnableImgui)
		{
			m_ImguiLayer = new ImguiLayer;
			PushLayer(m_ImguiLayer);
		}
	}

	Application::~Application()
	{
		for (Layer* layer : m_Layers)
		{
			layer->OnDetach();
			delete layer;
		}

		Audio::Shutdown();
		Renderer::Shutdown();

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
			ExecuteMainThreadQueue();
			m_Window->ProcessEvents();
			UpdatePhase(timer.Mark());
			if (!m_AppMinimized)
			{
				RenderPhase();
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
		Renderer::BeginFrame();

		if (m_Specification.EnableImgui)
			m_ImguiLayer->BeginFrame();

		for (Layer* layer : m_Layers)
		{
			layer->OnRender(); 
			layer->OnUIRender();
		}
			
		if (m_Specification.EnableImgui)
			m_ImguiLayer->EndFrame();

		Renderer::EndFrame();
	}

	void Application::SubmitToMainThread(const std::function<void()>& function)
	{
		std::scoped_lock<std::mutex> lock(m_Mutex);

		m_MainThreadQueue.emplace_back(function);
	}

	void Application::ExecuteMainThreadQueue()
	{
		std::scoped_lock<std::mutex> lock(m_Mutex);

		for (auto& function : m_MainThreadQueue)
			function();

		m_MainThreadQueue.clear();
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
			return false;
		});

		dispatcher.Dispatch<WindowResizeEvent>([&](WindowResizeEvent& e)
		{
			Renderer::OnResize(e.GetWidth(), e.GetHeight());
			return false;
		});
	}
}
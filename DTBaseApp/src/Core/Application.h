#pragma once
#include "Layer.h"
#include "Window.h"
#include "Renderer/GraphicsContext.h"
#include "Layers/ImguiLayer.h"

namespace DT
{
	struct ApplicationSpecification
	{
		WindowSpecification WindowSpecification;
		std::filesystem::path WorkingDirectory;
		bool EnableImgui = true;
	};

	class Application
	{
	public:
		Application(const ApplicationSpecification& specification);
		~Application();

		void PushLayer(Layer* layer);

		void Run();
		void CloseApplication() { m_AppRunning = false; }

		void OnEvent(Event& event);

		Window& GetWindow() { return *m_Window; }

		static Application& Get() { return *s_Instance; }

		void SubmitToMainThread(const std::function<void()>& function);
	private:
		void UpdatePhase(float dt);
		void RenderPhase();
		void ExecuteMainThreadQueue();
	private:
		bool m_AppRunning = true;
		bool m_AppMinimized = false;

		Window* m_Window = nullptr;
		std::vector<Layer*> m_Layers;
		ImguiLayer* m_ImguiLayer = nullptr;
		GraphicsContext* m_GraphicsContext;

		std::vector<std::function<void()>> m_MainThreadQueue;
		std::mutex m_Mutex;

		ApplicationSpecification m_Specification;
		inline static Application* s_Instance = nullptr;
	};
}
#pragma once
#include "Layer.h"
#include "Window.h"
#include "Renderer/RendererContext.h"

namespace DT
{
	struct ApplicationSpecification
	{
		WindowSpecification WindowSpecification;
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
	private:
		void UpdatePhase(float dt);
		void RenderPhase();
	private:
		bool m_AppRunning = true;

		Ref<RendererContext> m_RendererContext;
		Ref<Window> m_Window;
		std::vector<Layer*> m_Layers;

		ApplicationSpecification m_Specification;
		inline static Application* s_Instance = nullptr;
	};
}
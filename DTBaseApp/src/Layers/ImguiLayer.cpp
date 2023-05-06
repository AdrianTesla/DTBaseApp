#include "ImguiLayer.h"
#include "imgui.h"
#include "backends/imgui_impl_dx11.h"
#include "backends/imgui_impl_glfw.h"
#include "Core/Application.h"

namespace DT
{
	void ImguiLayer::OnAttach()
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGui::StyleColorsDark();
		ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;

		ImGui_ImplGlfw_InitForOpenGL((GLFWwindow*)Application::Get().GetWindow().GetPlatformWindow(), true);
		ImGui_ImplDX11_Init(GraphicsContext::GetDevice(), GraphicsContext::GetContext());
	}

	void ImguiLayer::OnDetach()
	{
		ImGui_ImplGlfw_Shutdown();
		ImGui_ImplDX11_Shutdown();
		ImGui::DestroyContext();
	}

	void ImguiLayer::BeginFrame()
	{
		ImGui_ImplDX11_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
	}

	void ImguiLayer::EndFrame()
	{
		auto rtv = GraphicsContext::GetSwapchainRTV();
		GraphicsContext::GetContext()->OMSetRenderTargets(1u, &rtv, nullptr);
		ImGui::Render();
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	}
}
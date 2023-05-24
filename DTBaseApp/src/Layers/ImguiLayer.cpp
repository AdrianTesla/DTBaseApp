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
		//ImGui::StyleColorsDark();
		ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;

		ImGui_ImplGlfw_InitForOpenGL((GLFWwindow*)Application::Get().GetWindow().GetPlatformWindow(), true);
		ImGui_ImplDX11_Init(GraphicsContext::GetDevice(), GraphicsContext::GetContext());

		// Imposta lo stile di ImGui in modalità scura con colori simili a Visual Studio, con rosso Vulkan e giallo

		ImGuiStyle& style = ImGui::GetStyle();

		style.Colors[ImGuiCol_Text] = ImVec4(0.86f, 0.86f, 0.86f, 1.0f);
		style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.6f, 0.6f, 0.6f, 1.0f);
		style.Colors[ImGuiCol_WindowBg] = ImVec4(0.15f, 0.15f, 0.15f, 1.0f);
		style.Colors[ImGuiCol_ChildBg] = ImVec4(0.2f, 0.2f, 0.2f, 0.0f);
		style.Colors[ImGuiCol_PopupBg] = ImVec4(0.1f, 0.1f, 0.1f, 0.92f);
		style.Colors[ImGuiCol_Border] = ImVec4(0.3f, 0.3f, 0.3f, 0.5f);
		style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
		style.Colors[ImGuiCol_FrameBg] = ImVec4(0.2f, 0.2f, 0.2f, 1.0f);
		style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.3f, 0.3f, 0.3f, 1.0f);
		style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.15f, 0.15f, 0.15f, 1.0f);
		style.Colors[ImGuiCol_TitleBg] = ImVec4(0.1f, 0.1f, 0.1f, 1.0f);
		style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.15f, 0.15f, 0.15f, 1.0f);
		style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.2f, 0.2f, 0.2f, 0.75f);
		style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.15f, 0.15f, 0.15f, 1.0f);
		style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.1f, 0.1f, 0.1f, 0.6f);
		style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.45f, 0.45f, 0.45f, 1.0f);
		style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.6f, 0.6f, 0.6f, 1.0f);
		style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.75f, 0.75f, 0.75f, 1.0f);
		style.Colors[ImGuiCol_CheckMark] = ImVec4(0.65f, 0.65f, 1.0f, 1.0f);
		style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.8f, 0.3f, 0.3f, 1.0f);
		style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.9f, 0.35f, 0.35f, 1.0f);
		style.Colors[ImGuiCol_Button] = ImVec4(0.15f, 0.15f, 0.15f, 1.0f);
		style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.3f, 0.3f, 0.3f, 1.0f);
		style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.2f, 0.2f, 0.2f, 1.0f);
		style.Colors[ImGuiCol_Header] = ImVec4(0.2f, 0.2f, 0.2f, 0.55f);
		style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.3f, 0.3f, 0.3f, 0.8f);
		style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.15f, 0.15f, 0.15f, 0.8f);
		style.Colors[ImGuiCol_Separator] = ImVec4(0.3f, 0.3f, 0.3f, 0.5f);
		style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.4f, 0.4f, 0.4f, 0.78f);
		style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
		style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.5f, 0.5f, 0.5f, 0.5f);
		style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.78f, 0.78f, 0.78f, 0.67f);
		style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.9f, 0.9f, 0.9f, 0.95f);
		style.Colors[ImGuiCol_Tab] = ImVec4(0.15f, 0.15f, 0.15f, 1.0f);
		style.Colors[ImGuiCol_TabHovered] = ImVec4(0.38f, 0.38f, 0.38f, 1.0f);
		style.Colors[ImGuiCol_TabActive] = ImVec4(0.28f, 0.28f, 0.28f, 1.0f);
		style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.15f, 0.15f, 0.15f, 1.0f);
		style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.2f, 0.2f, 0.2f, 1.0f);
		style.Colors[ImGuiCol_PlotLines] = ImVec4(0.8f, 0.3f, 0.3f, 1.0f);
		style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.0f, 0.65f, 0.0f, 1.0f);
		style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.8f, 0.3f, 0.3f, 1.0f);
		style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.0f, 0.65f, 0.0f, 1.0f);
		style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.4f, 0.4f, 0.4f, 0.5f);
		style.Colors[ImGuiCol_DragDropTarget] = ImVec4(0.4f, 0.4f, 0.4f, 1.0f);
		style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
		style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(0.7f, 0.7f, 0.7f, 0.7f);

		style.WindowRounding = 0.0f;
		style.FrameRounding = 0.0f;
		style.ScrollbarRounding = 0.0f;
		style.GrabRounding = 0.0f;
		style.TabRounding = 0.0f; 
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
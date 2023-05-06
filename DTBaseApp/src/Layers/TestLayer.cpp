#include "TestLayer.h"
#include "Core/Application.h"
#include "Renderer/Renderer.h"
#include "Renderer/Renderer2D.h"
#include "imgui.h"
#include "Core/Input.h"

namespace DT
{
	void TestLayer::OnAttach()
	{
		PipelineSpecification geoPipelineSpec{};
		geoPipelineSpec.PixelShaderPath = "QuadPS.cso";
		geoPipelineSpec.VertexShaderPath = "QuadVS.cso";

		FramebufferSpecification geoFramebufferSpec{};
		geoFramebufferSpec.Format = ImageFormat::RGBA8;
		geoFramebufferSpec.SwapchainTarget = false;
		m_GeoFramebuffer = CreateRef<Framebuffer>(geoFramebufferSpec);

		RenderPassSpecification geoPassSpecification{};
		geoPassSpecification.ClearColor = { 0.3f, 0.5f, 0.2f, 1.0f };
		geoPassSpecification.Pipeline = CreateRef<Pipeline>(geoPipelineSpec);
		geoPassSpecification.TargetFramebuffer = m_GeoFramebuffer;
		m_GeoRenderPass = CreateRef<RenderPass>(geoPassSpecification);

		RenderPassSpecification compositePassSpec{};
		compositePassSpec.ClearColor = { 0.2f, 0.2f, 0.7f, 1.0f };
		compositePassSpec.Pipeline = CreateRef<Pipeline>(geoPipelineSpec);
		compositePassSpec.TargetFramebuffer = CreateRef<Framebuffer>(FramebufferSpecification{});
		m_CompositeRenderPass = CreateRef<RenderPass>(compositePassSpec);
	}

	void TestLayer::OnUpdate(float dt)
	{
	}

	void TestLayer::OnEvent(Event& event)
	{
	}

	void TestLayer::OnDetach()
	{
	}

	void TestLayer::OnRender()
	{
		Renderer::BeginRenderPass(m_GeoRenderPass);
		Renderer::EndRenderPass();

		Renderer::BeginRenderPass(m_CompositeRenderPass);
		Renderer::EndRenderPass();
	}

	void TestLayer::OnUIRender()
	{
		ImGui::Begin("Test");
		ImGui::Image(m_GeoFramebuffer->GetImage()->GetSRV(), { 160.0f, 90.0f });
		ImGui::End();
	}
}
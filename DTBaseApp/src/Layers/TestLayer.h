#pragma once
#include "Core/Layer.h"
#include "Renderer/Framebuffer.h"
#include "Renderer/RenderPass.h"
#include "Renderer/DX11Buffers.h"
#include "Renderer/Pipeline.h"
#include "Renderer/Texture.h"
#include "Renderer/ParticleSystem.h"

namespace DT
{
	class TestLayer : public Layer
	{
	public:
		virtual void OnAttach() override;
		virtual void OnUpdate(float dt) override;
		virtual void OnEvent(Event& event) override;
		virtual void OnDetach() override;
		virtual void OnRender() override;
		virtual void OnUIRender() override;
	private:
		Ref<RenderPass> m_GeoRenderPass;
		Ref<Framebuffer> m_GeoFramebuffer;

		Ref<RenderPass> m_CompositeRenderPass;
	};
}
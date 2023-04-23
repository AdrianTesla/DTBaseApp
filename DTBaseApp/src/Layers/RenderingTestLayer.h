#pragma once
#include "Core/Layer.h"
#include "Renderer/Framebuffer.h"
#include "Renderer/RenderPass.h"

namespace DT
{
	class RenderingTestLayer : public Layer
	{
	public:
		virtual void OnAttach() override;
		virtual void OnUpdate(float dt) override;
		virtual void OnRender() override;
		virtual void OnEvent(Event& event) override;
		virtual void OnDetach() override;
	private:
		Ref<Framebuffer> m_Framebuffer;
		Ref<RenderPass> m_RenderPass;
	};
}
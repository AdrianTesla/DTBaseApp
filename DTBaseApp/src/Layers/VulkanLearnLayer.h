#pragma once
#include "Core/Layer.h"

namespace DT
{
	class VulkanLearnLayer : public Layer
	{
	public:
		virtual void OnAttach() override;
		virtual void OnUpdate(float dt) override;
		virtual void OnEvent(Event& event) override;
		virtual void OnDetach() override;
	private:
		float m_Opacity = 1.0f;
	};
}
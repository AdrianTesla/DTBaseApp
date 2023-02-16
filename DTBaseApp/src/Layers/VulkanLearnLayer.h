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
		uint32 m_TimeStepIndex = 0u;
		std::array<float, 60> m_TimeSteps;
	};
}
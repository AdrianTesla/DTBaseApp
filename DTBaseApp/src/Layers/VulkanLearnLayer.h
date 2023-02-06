#pragma once
#include "Core/Layer.h"

namespace DT
{
	class VulkanLearnLayer : public Layer
	{
	public:
		virtual void OnAttach() override;
		virtual void OnUpdate(float dt) override;
		virtual void OnDetach() override;
	};
}
#pragma once
#include "Core/Layer.h"

namespace DT
{
	class TestLayer : public Layer
	{
	public:
		virtual void OnAttach() override;
		virtual void OnUpdate(float dt) override;
		virtual void OnEvent(Event& event) override;
		virtual void OnDetach() override;
	};
}
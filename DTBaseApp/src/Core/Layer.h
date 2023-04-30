#pragma once
#include "Core.h"
#include "Event.h"

namespace DT
{
	class Layer
	{
	public:
		Layer() = default;
		virtual ~Layer() = default;

		virtual void OnAttach() {}
		virtual void OnDetach() {}
		virtual void OnUpdate(float dt) {}
		virtual void OnRender() {}
		virtual void OnUIRender() {}
		virtual void OnEvent(Event& event) {}
	};
}
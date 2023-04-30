#pragma once
#include "Core/Layer.h"

namespace DT
{
	class ImguiLayer : public Layer
	{
	public:
		virtual void OnAttach() override;
		virtual void OnDetach() override;

		void BeginFrame();
		void EndFrame();
	};
}

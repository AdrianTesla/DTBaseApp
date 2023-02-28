#pragma once
#include "Core/Core.h"
#include "Platform/API_Vulkan/Vulkan.h"

namespace DT
{
	class Renderer
	{
	public:
		static void Init();
		static void Shutdown();

		static void BeginFrame();
		static void EndFrame();

		static void OnWindowResize();

		static void SetVerticalSync(bool enabled);
		static uint32 CurrentFrame();
	};
}
#pragma once
#include "Vulkan.h"
#include "Core/Window.h"

namespace DT
{
	class VulkanSwapchain
	{
	public:
		void Init();
		void Shutdown();

		void Resize(int32 width, int32 height);
		void Present();
	private:
		VkSurfaceKHR m_Surface = VK_NULL_HANDLE;
	};
}
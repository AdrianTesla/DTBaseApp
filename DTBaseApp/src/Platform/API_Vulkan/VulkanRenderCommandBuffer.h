#pragma once
#include "Vulkan.h"

namespace DT
{
	class VulkanRenderCommandBuffer : public RefCounted
	{
	public:
		VulkanRenderCommandBuffer();
		~VulkanRenderCommandBuffer();

		void Begin();
		void End();

		VkCommandBuffer GetActiveCommandBuffer() const;
	private:
		InFlight<VkCommandBuffer> m_CommandBuffers;
	};
}
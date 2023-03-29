#pragma once
#include "Core/Layer.h"
#include "Platform/API_Vulkan/VulkanPipeline.h"
#include "Platform/API_Vulkan/VulkanResources.h"
#include "Platform/API_Vulkan/VulkanImage.h"

namespace DT
{
	class VulkanComputeLayer : public Layer
	{
	public:
		virtual void OnAttach() override;
		virtual void OnUpdate(float dt) override;
		virtual void OnRender() override;
		virtual void OnEvent(Event& event) override;
		virtual void OnDetach() override;
	private:
		InFlight<VkCommandBuffer> m_CommandBuffers;
		Ref<VulkanVertexBuffer> m_VertexBuffer;
		Ref<VulkanPipeline> m_Pipeline;
	};
}
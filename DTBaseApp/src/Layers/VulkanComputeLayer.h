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
		VkDescriptorPool m_DescriptorPool = VK_NULL_HANDLE;
		InFlight<VkDescriptorSet> m_DescriptorSets{};

		Ref<VulkanComputePipeline> m_ComputePipeline;

		InFlight<VulkanBuffer> m_ParticleBuffer{};
		uint64 m_ParticleBufferSize;
		InFlight<VkCommandBuffer> m_GraphicsCommandBuffers;

		Ref<VulkanUniformBuffer> m_UniformBuffer;
	};
}
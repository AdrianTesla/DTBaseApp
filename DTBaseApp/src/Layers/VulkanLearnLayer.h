#pragma once
#include "Core/Layer.h"
#include "Platform/API_Vulkan/VulkanPipeline.h"
#include "Platform/API_Vulkan/VulkanBuffers.h"
#include "Platform/API_Vulkan/VulkanImage.h"

namespace DT
{
	struct Pixel
	{
		uint8 r;
		uint8 g;
		uint8 b;
		uint8 a;
	};

	class VulkanLearnLayer : public Layer
	{
	public:
		virtual void OnAttach() override;
		virtual void OnUpdate(float dt) override;
		virtual void OnRender() override;
		virtual void OnEvent(Event& event) override;
		virtual void OnDetach() override;
	private:
		void CreateCommandBuffers();
		void CreateBuffers();
		void UpdateUniformBuffers();
		void CreateSampler();
		void CreateImage();

		void CreateDescriptorPools();
		void CreateDescriptorSets();

		void RecordCommandBuffer(VkCommandBuffer commandBuffer);
		void ExecuteCommandBuffer(VkCommandBuffer commandBuffer);
	private:
		Ref<VulkanShader> m_Shader;
		Ref<VulkanPipeline> m_Pipeline;
		Ref<VulkanVertexBuffer> m_VertexBuffer;
		Ref<VulkanIndexBuffer> m_IndexBuffer;

		InFlight<VkCommandBuffer> m_GraphicsCommandBuffers;

		InFlight<Ref<VulkanUniformBuffer>> m_UniformBuffers;

		VkDescriptorPool m_DescriptorPool = VK_NULL_HANDLE;
		InFlight<VkDescriptorSet> m_DescriptorSets;

		Ref<VulkanTexture2D> m_Texture;

		VkSampler m_Sampler = VK_NULL_HANDLE;
	};
}
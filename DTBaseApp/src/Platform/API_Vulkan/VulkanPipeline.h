#pragma once
#include "Core/Ref.h"
#include "Vulkan.h"
#include "VulkanShader.h"

namespace DT
{
	struct PipelineSpecification
	{
		Ref<VulkanShader> Shader;
		PolygonMode       PolygonMode = PolygonMode::Fill;
		PrimitiveTopology Topology    = PrimitiveTopology::TriangleList;
		FaceCulling       Culling     = FaceCulling::None;
	};

	class VulkanPipeline : public RefCounted
	{
	public:
		VulkanPipeline(const PipelineSpecification& specification);
		~VulkanPipeline();

		void Invalidate();
		void Destroy();

		VkDescriptorSetLayout& GetDescriptorSetLayout() { return m_DescriptorSetLayout; }
		VkPipelineLayout GetPipelineLayout() const { return m_PipelineLayout; }
		VkPipeline GetVulkanPipeline() const { return m_Pipeline; }
	private:
		PipelineSpecification m_Specification;

		VkPipeline m_Pipeline = VK_NULL_HANDLE;
		VkDescriptorSetLayout m_DescriptorSetLayout = VK_NULL_HANDLE;
		VkPipelineLayout m_PipelineLayout = VK_NULL_HANDLE;
	};

	class VulkanComputePipeline : public RefCounted
	{
	public:
		VulkanComputePipeline();
		~VulkanComputePipeline();

		void Invalidate();
		void Destroy();

		VkDescriptorSetLayout& GetDescriptorSetLayout() { return m_DescriptorSetLayout; }
		VkPipelineLayout GetPipelineLayout() const { return m_PipelineLayout; }
		VkPipeline GetVulkanPipeline() const { return m_Pipeline; }
	private:
		VkPipeline m_Pipeline = VK_NULL_HANDLE;
		VkPipelineLayout m_PipelineLayout = VK_NULL_HANDLE;
		VkDescriptorSetLayout m_DescriptorSetLayout = VK_NULL_HANDLE;
	};
}
#pragma once
#include "Core/Ref.h"
#include "Vulkan.h"
#include "VulkanShader.h"

namespace DT
{
	enum class PolygonMode
	{
		Fill,
		Wireframe,
		Point
	};

	struct PipelineSpecification
	{
		Ref<VulkanShader> Shader;
		PolygonMode PolygonMode = PolygonMode::Fill;
	};

	class VulkanPipeline : public RefCounted
	{
	public:
		VulkanPipeline(const PipelineSpecification& specification);
		~VulkanPipeline();

		void Invalidate();
		void Destroy();

		VkPipeline GetVulkanPipeline() const { return m_Pipeline; }
		VkPipelineLayout GetPipelineLayout() const { return m_PipelineLayout; }
	private:
		PipelineSpecification m_Specification;
		VkPipeline m_Pipeline = VK_NULL_HANDLE;
		VkPipelineLayout m_PipelineLayout = VK_NULL_HANDLE;
	};

	class VulkanComputePipeline : public RefCounted
	{
	public:
		VulkanComputePipeline();
		~VulkanComputePipeline();
	private:
		VkPipeline m_Pipeline;
	};

	class VulkanRayTracingPipeline : public RefCounted
	{
	public:
		VulkanRayTracingPipeline();
		~VulkanRayTracingPipeline();
	private:
		VkPipeline m_Pipeline;
	};
}
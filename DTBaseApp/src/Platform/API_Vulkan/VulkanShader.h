#pragma once
#include "Vulkan.h"

namespace DT
{
	struct ShaderSpecification
	{
		std::filesystem::path VertexSpirvPath;
		std::filesystem::path FragmentSpirvPath;
	};

	class VulkanShader : public RefCounted
	{
	public:
		VulkanShader(const ShaderSpecification& specification);
		~VulkanShader();

		void Invalidate();
		void Destroy();

		const std::array<VkPipelineShaderStageCreateInfo, 2u>& GetPipelineShaderStageCreateInfos() const { return m_PipelineShaderStages; }
	private:
		std::array<VkPipelineShaderStageCreateInfo, 2u> m_PipelineShaderStages;
		ShaderSpecification m_Specification;
	};
}
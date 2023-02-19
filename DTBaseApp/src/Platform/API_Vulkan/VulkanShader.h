#pragma once
#include "Vulkan.h"

namespace DT
{
	class VulkanShader : public RefCounted
	{
	public:
		VulkanShader();
		~VulkanShader();

		void Invalidate();
		void Destroy();

		const std::array<VkPipelineShaderStageCreateInfo, 2u>& GetPipelineShaderStageCreateInfos() const { return m_PipelineShaderStages; }
	private:
		std::array<VkPipelineShaderStageCreateInfo, 2u> m_PipelineShaderStages;
	};
}
#include "VulkanShader.h"
#include "VulkanContext.h"

namespace DT
{
	VulkanShader::VulkanShader(const ShaderSpecification& specification)
		: m_Specification(specification)
	{
		Invalidate();
	}

	VulkanShader::~VulkanShader()
	{
		Destroy();
	}

	void VulkanShader::Invalidate()
	{
		VkDevice device = VulkanContext::GetCurrentVulkanDevice();

		// vertex shader
		{
			VkShaderModuleCreateInfo shaderModuleCreateInfo{};
			Buffer spirv = FileSystem::ReadFileBinary(m_Specification.VertexSpirvPath);
			shaderModuleCreateInfo.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			shaderModuleCreateInfo.pNext    = nullptr;
			shaderModuleCreateInfo.flags    = 0u;
			shaderModuleCreateInfo.codeSize = spirv.Size;
			shaderModuleCreateInfo.pCode    = (uint32*)spirv.Data;
			VK_CALL(vkCreateShaderModule(device, &shaderModuleCreateInfo, nullptr, &m_PipelineShaderStages[0].module));
			spirv.Release();

			m_PipelineShaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			m_PipelineShaderStages[0].pNext = nullptr;
			m_PipelineShaderStages[0].flags = 0u;
			m_PipelineShaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
			m_PipelineShaderStages[0].pName = "main";
			m_PipelineShaderStages[0].pSpecializationInfo = nullptr;
		}

		// fragment shader
		{
			VkShaderModuleCreateInfo shaderModuleCreateInfo{}; 
			Buffer spirv = FileSystem::ReadFileBinary(m_Specification.FragmentSpirvPath);
			shaderModuleCreateInfo.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			shaderModuleCreateInfo.pNext    = nullptr;
			shaderModuleCreateInfo.flags    = 0u;
			shaderModuleCreateInfo.codeSize = spirv.Size;
			shaderModuleCreateInfo.pCode    = (uint32*)spirv.Data;
			VK_CALL(vkCreateShaderModule(device, &shaderModuleCreateInfo, nullptr, &m_PipelineShaderStages[1].module));
			spirv.Release();

			m_PipelineShaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			m_PipelineShaderStages[1].pNext = nullptr;
			m_PipelineShaderStages[1].flags = 0u;
			m_PipelineShaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
			m_PipelineShaderStages[1].pName = "main";
			m_PipelineShaderStages[1].pSpecializationInfo = nullptr;
		}
	}

	void VulkanShader::Destroy()
	{
		VkDevice device = VulkanContext::GetCurrentVulkanDevice();
		for (size_t i = 0u; i < m_PipelineShaderStages.size(); i++)
			vkDestroyShaderModule(device, m_PipelineShaderStages[i].module, nullptr);
	}
}

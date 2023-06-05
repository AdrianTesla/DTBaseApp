#include "VulkanPipeline.h"
#include "VulkanContext.h"
#include "VulkanRenderer.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace DT
{

	#pragma region VulkanGraphicsPipeline

	VulkanPipeline::VulkanPipeline(const PipelineSpecification& specification)
		: m_Specification(specification)
	{
		Invalidate();
	}

	void VulkanPipeline::Invalidate()
	{
		Destroy();

		Timer timer;
		VkDevice device = VulkanContext::GetCurrentVulkanDevice();

		struct Vertex
		{
			glm::vec3 Position;
			glm::vec2 TexCoord;
		};

		VkVertexInputBindingDescription vertexInputBindingDescription{};
		vertexInputBindingDescription.binding   = 0u;
		vertexInputBindingDescription.stride    = sizeof(Vertex);
		vertexInputBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		VkVertexInputAttributeDescription vertexInputAttributeDescriptions[2];
		vertexInputAttributeDescriptions[0].location = 0u;
		vertexInputAttributeDescriptions[0].binding  = 0u;
		vertexInputAttributeDescriptions[0].format   = VK_FORMAT_R32G32B32_SFLOAT;
		vertexInputAttributeDescriptions[0].offset   = offsetof(Vertex, Position);

		vertexInputAttributeDescriptions[1].location = 1u;
		vertexInputAttributeDescriptions[1].binding  = 0u;
		vertexInputAttributeDescriptions[1].format   = VK_FORMAT_R32G32_SFLOAT;
		vertexInputAttributeDescriptions[1].offset   = offsetof(Vertex, TexCoord);

		VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo{};
		pipelineVertexInputStateCreateInfo.sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		pipelineVertexInputStateCreateInfo.pNext                           = nullptr;
		pipelineVertexInputStateCreateInfo.flags                           = 0u;
		pipelineVertexInputStateCreateInfo.vertexBindingDescriptionCount   = 1u;
		pipelineVertexInputStateCreateInfo.pVertexBindingDescriptions      = &vertexInputBindingDescription;
		pipelineVertexInputStateCreateInfo.vertexAttributeDescriptionCount = (uint32)std::size(vertexInputAttributeDescriptions);
		pipelineVertexInputStateCreateInfo.pVertexAttributeDescriptions    = vertexInputAttributeDescriptions;

		VkPipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo{};
		pipelineInputAssemblyStateCreateInfo.sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		pipelineInputAssemblyStateCreateInfo.pNext                  = nullptr;
		pipelineInputAssemblyStateCreateInfo.flags                  = 0u;
		pipelineInputAssemblyStateCreateInfo.topology               = Convert::ToVulkanPrimitiveTopology(m_Specification.Topology);
		pipelineInputAssemblyStateCreateInfo.primitiveRestartEnable = VK_FALSE;

		VkPipelineTessellationStateCreateInfo pipelineTessellationStateCreateInfo{};
		pipelineTessellationStateCreateInfo.sType              = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
		pipelineTessellationStateCreateInfo.pNext              = nullptr;
		pipelineTessellationStateCreateInfo.flags              = 0u;
		pipelineTessellationStateCreateInfo.patchControlPoints = 1u;

		VkPipelineViewportStateCreateInfo pipelineViewportStateCreateInfo{};
		pipelineViewportStateCreateInfo.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		pipelineViewportStateCreateInfo.pNext         = nullptr;
		pipelineViewportStateCreateInfo.flags         = 0u;
		pipelineViewportStateCreateInfo.viewportCount = 1u;
		pipelineViewportStateCreateInfo.pViewports    = nullptr;
		pipelineViewportStateCreateInfo.scissorCount  = 1u;
		pipelineViewportStateCreateInfo.pScissors     = nullptr;

		VkPipelineRasterizationStateCreateInfo pipelineRasterizationStateCreateInfo{};
		pipelineRasterizationStateCreateInfo.sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		pipelineRasterizationStateCreateInfo.pNext                   = nullptr;
		pipelineRasterizationStateCreateInfo.flags                   = 0u;
		pipelineRasterizationStateCreateInfo.depthClampEnable        = VK_FALSE;
		pipelineRasterizationStateCreateInfo.rasterizerDiscardEnable = VK_FALSE;
		pipelineRasterizationStateCreateInfo.polygonMode             = Convert::ToVulkanPolygonMode(m_Specification.PolygonMode);
		pipelineRasterizationStateCreateInfo.cullMode                = Convert::ToVulkanCullMode(m_Specification.Culling);
		pipelineRasterizationStateCreateInfo.frontFace               = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		pipelineRasterizationStateCreateInfo.depthBiasEnable         = VK_FALSE;
		pipelineRasterizationStateCreateInfo.depthBiasConstantFactor = 0.0f;
		pipelineRasterizationStateCreateInfo.depthBiasClamp          = 0.0f;
		pipelineRasterizationStateCreateInfo.depthBiasSlopeFactor    = 0.0f;
		pipelineRasterizationStateCreateInfo.lineWidth               = 2.0f;

		VkPipelineMultisampleStateCreateInfo pipelineMultisampleStateCreateInfo{};
		pipelineMultisampleStateCreateInfo.sType                 = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		pipelineMultisampleStateCreateInfo.pNext                 = nullptr;
		pipelineMultisampleStateCreateInfo.flags                 = 0u;
		pipelineMultisampleStateCreateInfo.rasterizationSamples  = VulkanRenderer::GetSwapchain().GetSwapchainSampleCount();
		pipelineMultisampleStateCreateInfo.sampleShadingEnable   = VK_FALSE;
		pipelineMultisampleStateCreateInfo.minSampleShading      = 0.0f;
		pipelineMultisampleStateCreateInfo.pSampleMask           = nullptr;
		pipelineMultisampleStateCreateInfo.alphaToCoverageEnable = VK_FALSE;
		pipelineMultisampleStateCreateInfo.alphaToOneEnable      = VK_FALSE;

		VkPipelineDepthStencilStateCreateInfo pipelineDepthStencilStateCreateInfo{};
		pipelineDepthStencilStateCreateInfo.sType                 = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		pipelineDepthStencilStateCreateInfo.pNext                 = nullptr;
		pipelineDepthStencilStateCreateInfo.flags                 = 0u;
		pipelineDepthStencilStateCreateInfo.depthTestEnable       = VK_TRUE;
		pipelineDepthStencilStateCreateInfo.depthWriteEnable      = VK_TRUE;
		pipelineDepthStencilStateCreateInfo.depthCompareOp        = VK_COMPARE_OP_LESS;
		pipelineDepthStencilStateCreateInfo.depthBoundsTestEnable = VK_FALSE;
		pipelineDepthStencilStateCreateInfo.stencilTestEnable     = VK_FALSE;
		pipelineDepthStencilStateCreateInfo.front.failOp          = VK_STENCIL_OP_KEEP;
		pipelineDepthStencilStateCreateInfo.front.passOp          = VK_STENCIL_OP_KEEP;
		pipelineDepthStencilStateCreateInfo.front.depthFailOp     = VK_STENCIL_OP_KEEP;
		pipelineDepthStencilStateCreateInfo.front.compareOp       = VK_COMPARE_OP_EQUAL;
		pipelineDepthStencilStateCreateInfo.front.compareMask     = 0u;
		pipelineDepthStencilStateCreateInfo.front.writeMask       = 0u;
		pipelineDepthStencilStateCreateInfo.front.reference       = 0u;
		pipelineDepthStencilStateCreateInfo.back.failOp           = VK_STENCIL_OP_KEEP;
		pipelineDepthStencilStateCreateInfo.back.passOp           = VK_STENCIL_OP_KEEP;
		pipelineDepthStencilStateCreateInfo.back.depthFailOp      = VK_STENCIL_OP_KEEP;
		pipelineDepthStencilStateCreateInfo.back.compareOp        = VK_COMPARE_OP_EQUAL;
		pipelineDepthStencilStateCreateInfo.back.compareMask      = 0u;
		pipelineDepthStencilStateCreateInfo.back.writeMask        = 0u;
		pipelineDepthStencilStateCreateInfo.back.reference        = 0u;
		pipelineDepthStencilStateCreateInfo.minDepthBounds        = 0.0f;
		pipelineDepthStencilStateCreateInfo.maxDepthBounds        = 1.0f;

		VkPipelineColorBlendAttachmentState pipelineColorBlendAttachmentStates[1];
		for (size_t i = 0u; i < std::size(pipelineColorBlendAttachmentStates); i++)
		{
			pipelineColorBlendAttachmentStates[i].blendEnable         = VK_FALSE;
			pipelineColorBlendAttachmentStates[i].srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
			pipelineColorBlendAttachmentStates[i].dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
			pipelineColorBlendAttachmentStates[i].colorBlendOp        = VK_BLEND_OP_ADD;
			pipelineColorBlendAttachmentStates[i].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
			pipelineColorBlendAttachmentStates[i].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
			pipelineColorBlendAttachmentStates[i].alphaBlendOp        = VK_BLEND_OP_ADD;
			pipelineColorBlendAttachmentStates[i].colorWriteMask      = 
				VK_COLOR_COMPONENT_R_BIT | 
				VK_COLOR_COMPONENT_G_BIT |
				VK_COLOR_COMPONENT_B_BIT |
				VK_COLOR_COMPONENT_A_BIT;
		}

		VkPipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo{};
		pipelineColorBlendStateCreateInfo.sType             = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		pipelineColorBlendStateCreateInfo.pNext             = nullptr;
		pipelineColorBlendStateCreateInfo.flags             = 0u;
		pipelineColorBlendStateCreateInfo.logicOpEnable     = VK_FALSE;
		pipelineColorBlendStateCreateInfo.logicOp           = VK_LOGIC_OP_NO_OP;
		pipelineColorBlendStateCreateInfo.attachmentCount   = (uint32)std::size(pipelineColorBlendAttachmentStates);
		pipelineColorBlendStateCreateInfo.pAttachments      = pipelineColorBlendAttachmentStates;
		pipelineColorBlendStateCreateInfo.blendConstants[0] = 0.0f;
		pipelineColorBlendStateCreateInfo.blendConstants[1] = 0.0f;
		pipelineColorBlendStateCreateInfo.blendConstants[2] = 0.0f;
		pipelineColorBlendStateCreateInfo.blendConstants[3] = 0.0f;

		VkDynamicState dynamicStates[] = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR
		};

		VkPipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo{};
		pipelineDynamicStateCreateInfo.sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		pipelineDynamicStateCreateInfo.pNext             = nullptr;
		pipelineDynamicStateCreateInfo.flags             = 0u;
		pipelineDynamicStateCreateInfo.dynamicStateCount = (uint32)std::size(dynamicStates);
		pipelineDynamicStateCreateInfo.pDynamicStates    = dynamicStates;

		VkDescriptorSetLayoutBinding descriptorSetLayoutBindings[2];
		descriptorSetLayoutBindings[0].binding            = 0u;
		descriptorSetLayoutBindings[0].descriptorType     = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorSetLayoutBindings[0].descriptorCount    = 1u;
		descriptorSetLayoutBindings[0].stageFlags         = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		descriptorSetLayoutBindings[0].pImmutableSamplers = nullptr;

		descriptorSetLayoutBindings[1].binding            = 1u;
		descriptorSetLayoutBindings[1].descriptorType     = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorSetLayoutBindings[1].descriptorCount    = 1u;
		descriptorSetLayoutBindings[1].stageFlags         = VK_SHADER_STAGE_FRAGMENT_BIT;
		descriptorSetLayoutBindings[1].pImmutableSamplers = nullptr;

		VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo{};
		descriptorSetLayoutCreateInfo.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		descriptorSetLayoutCreateInfo.pNext        = nullptr;
		descriptorSetLayoutCreateInfo.flags        = 0u;
		descriptorSetLayoutCreateInfo.bindingCount = (uint32)std::size(descriptorSetLayoutBindings);
		descriptorSetLayoutCreateInfo.pBindings    = descriptorSetLayoutBindings;
		VK_CALL(vkCreateDescriptorSetLayout(device, &descriptorSetLayoutCreateInfo, nullptr, &m_DescriptorSetLayout));

		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		pushConstantRange.offset     = 0u;
		pushConstantRange.size       = sizeof(glm::mat4);

		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
		pipelineLayoutCreateInfo.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutCreateInfo.pNext                  = nullptr;
		pipelineLayoutCreateInfo.flags                  = 0u;
		pipelineLayoutCreateInfo.setLayoutCount         = 1u;
		pipelineLayoutCreateInfo.pSetLayouts            = &m_DescriptorSetLayout;
		pipelineLayoutCreateInfo.pushConstantRangeCount = 1u;
		pipelineLayoutCreateInfo.pPushConstantRanges    = &pushConstantRange;
		VK_CALL(vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, nullptr, &m_PipelineLayout));

		VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo{};
		graphicsPipelineCreateInfo.sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		graphicsPipelineCreateInfo.pNext               = nullptr;
		graphicsPipelineCreateInfo.flags               = 0u;
		graphicsPipelineCreateInfo.stageCount          = (uint32)m_Specification.Shader->GetPipelineShaderStageCreateInfos().size();
		graphicsPipelineCreateInfo.pStages             = m_Specification.Shader->GetPipelineShaderStageCreateInfos().data();
		graphicsPipelineCreateInfo.pVertexInputState   = &pipelineVertexInputStateCreateInfo;
		graphicsPipelineCreateInfo.pInputAssemblyState = &pipelineInputAssemblyStateCreateInfo;
		graphicsPipelineCreateInfo.pTessellationState  = &pipelineTessellationStateCreateInfo;
		graphicsPipelineCreateInfo.pViewportState      = &pipelineViewportStateCreateInfo;
		graphicsPipelineCreateInfo.pRasterizationState = &pipelineRasterizationStateCreateInfo;
		graphicsPipelineCreateInfo.pMultisampleState   = &pipelineMultisampleStateCreateInfo;
		graphicsPipelineCreateInfo.pDepthStencilState  = &pipelineDepthStencilStateCreateInfo;
		graphicsPipelineCreateInfo.pColorBlendState    = &pipelineColorBlendStateCreateInfo;
		graphicsPipelineCreateInfo.pDynamicState       = &pipelineDynamicStateCreateInfo;
		graphicsPipelineCreateInfo.layout              = m_PipelineLayout;
		graphicsPipelineCreateInfo.renderPass          = VulkanRenderer::GetSwapchain().GetSwapchainRenderPass();
		graphicsPipelineCreateInfo.subpass             = 0u;
		graphicsPipelineCreateInfo.basePipelineHandle  = VK_NULL_HANDLE;
		graphicsPipelineCreateInfo.basePipelineIndex   = 0;
		VK_CALL(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1u, &graphicsPipelineCreateInfo, nullptr, &m_Pipeline));
	
		LOG_TRACE("Created graphics pipeline: {} ms", timer.ElapsedMilliseconds());
	}

	void VulkanPipeline::Destroy()
	{
		VkDevice device = VulkanContext::GetCurrentVulkanDevice();
		vkDestroyPipelineLayout(device, m_PipelineLayout, nullptr);
		vkDestroyDescriptorSetLayout(device, m_DescriptorSetLayout, nullptr);
		vkDestroyPipeline(device, m_Pipeline, nullptr);
		m_DescriptorSetLayout = VK_NULL_HANDLE;
		m_PipelineLayout = VK_NULL_HANDLE;
		m_Pipeline = VK_NULL_HANDLE;
	}

	VulkanPipeline::~VulkanPipeline()
	{
		Destroy();
	}

	#pragma endregion

	#pragma region VulkanComputePipeline

	VulkanComputePipeline::VulkanComputePipeline()
	{
		Invalidate();
	}

	VulkanComputePipeline::~VulkanComputePipeline()
	{
		Destroy();
	}

	void VulkanComputePipeline::Invalidate()
	{
		Destroy();

		VkDevice device = VulkanContext::GetCurrentVulkanDevice();
		
		Buffer spirv = FileSystem::ReadFileBinary("assets/shaders/comp.spv");

		VkShaderModule computeShaderModule{};
		VkShaderModuleCreateInfo shaderModuleCreateInfo{};
		shaderModuleCreateInfo.sType	= VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		shaderModuleCreateInfo.pNext	= nullptr;
		shaderModuleCreateInfo.flags	= 0u;
		shaderModuleCreateInfo.codeSize	= spirv.Size;
		shaderModuleCreateInfo.pCode	= (uint32*)spirv.Data;
		VK_CALL(vkCreateShaderModule(device, &shaderModuleCreateInfo, nullptr, &computeShaderModule));
		spirv.Release();

		VkDescriptorSetLayoutBinding descriptorSetLayoutBindings[3];
		descriptorSetLayoutBindings[0].binding			  = 0u;
		descriptorSetLayoutBindings[0].descriptorType	  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorSetLayoutBindings[0].descriptorCount	  = 1u;
		descriptorSetLayoutBindings[0].stageFlags		  = VK_SHADER_STAGE_COMPUTE_BIT;
		descriptorSetLayoutBindings[0].pImmutableSamplers = nullptr;

		descriptorSetLayoutBindings[1].binding			  = 1u;
		descriptorSetLayoutBindings[1].descriptorType	  = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		descriptorSetLayoutBindings[1].descriptorCount	  = 1u;
		descriptorSetLayoutBindings[1].stageFlags		  = VK_SHADER_STAGE_COMPUTE_BIT;
		descriptorSetLayoutBindings[1].pImmutableSamplers = nullptr;

		descriptorSetLayoutBindings[2].binding			  = 2u;
		descriptorSetLayoutBindings[2].descriptorType	  = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		descriptorSetLayoutBindings[2].descriptorCount	  = 1u;
		descriptorSetLayoutBindings[2].stageFlags		  = VK_SHADER_STAGE_COMPUTE_BIT;
		descriptorSetLayoutBindings[2].pImmutableSamplers = nullptr;

		VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo{};
		descriptorSetLayoutCreateInfo.sType		   = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		descriptorSetLayoutCreateInfo.pNext		   = nullptr;
		descriptorSetLayoutCreateInfo.flags		   = 0u;
		descriptorSetLayoutCreateInfo.bindingCount = (uint32)std::size(descriptorSetLayoutBindings);
		descriptorSetLayoutCreateInfo.pBindings	   = descriptorSetLayoutBindings;
		VK_CALL(vkCreateDescriptorSetLayout(device, &descriptorSetLayoutCreateInfo, nullptr, &m_DescriptorSetLayout));

		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
		pipelineLayoutCreateInfo.sType					= VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutCreateInfo.pNext					= nullptr;
		pipelineLayoutCreateInfo.flags					= 0u;
		pipelineLayoutCreateInfo.setLayoutCount			= 1u;
		pipelineLayoutCreateInfo.pSetLayouts			= &m_DescriptorSetLayout;
		pipelineLayoutCreateInfo.pushConstantRangeCount	= 0u;
		pipelineLayoutCreateInfo.pPushConstantRanges	= nullptr;
		VK_CALL(vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, nullptr, &m_PipelineLayout));

		VkComputePipelineCreateInfo computePipelineCreateInfo{};
		computePipelineCreateInfo.sType				          = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
		computePipelineCreateInfo.pNext				          = nullptr;
		computePipelineCreateInfo.flags				          = 0u;
		computePipelineCreateInfo.stage.sType				  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		computePipelineCreateInfo.stage.pNext				  = nullptr;
		computePipelineCreateInfo.stage.flags				  = 0u;
		computePipelineCreateInfo.stage.stage				  = VK_SHADER_STAGE_COMPUTE_BIT;
		computePipelineCreateInfo.stage.module			      = computeShaderModule;
		computePipelineCreateInfo.stage.pName				  = "main";
		computePipelineCreateInfo.stage.pSpecializationInfo   = nullptr;
		computePipelineCreateInfo.layout			          = m_PipelineLayout;
		computePipelineCreateInfo.basePipelineHandle          = 0u;
		computePipelineCreateInfo.basePipelineIndex	          = -1;
		VK_CALL(vkCreateComputePipelines(device, VK_NULL_HANDLE, 1u, &computePipelineCreateInfo, nullptr, &m_Pipeline));
		vkDestroyShaderModule(device, computeShaderModule, nullptr);
	}

	void VulkanComputePipeline::Destroy()
	{
		VkDevice device = VulkanContext::GetCurrentVulkanDevice();
		vkDestroyDescriptorSetLayout(device, m_DescriptorSetLayout, nullptr);
		vkDestroyPipelineLayout(device, m_PipelineLayout, nullptr);
		vkDestroyPipeline(device, m_Pipeline, nullptr);
		m_DescriptorSetLayout = VK_NULL_HANDLE;
		m_Pipeline = VK_NULL_HANDLE;
		m_PipelineLayout = VK_NULL_HANDLE;
	}

	#pragma endregion

}
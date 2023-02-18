#include "VulkanPipeline.h"
#include "VulkanContext.h"

namespace DT
{
	struct PushConstant
	{
		float AspectRatio;
		float Time;
	};

	VulkanPipeline::VulkanPipeline(const PipelineSpecification& specification)
		: m_Specification(specification)
	{
		Invalidate();
	}

	void VulkanPipeline::Invalidate()
	{
		VkDevice device = VulkanContext::GetCurrentVulkanDevice();

		Buffer vertSPIRV = FileSystem::ReadFileBinary("assets/shaders/vert.spv");
		Buffer fragSPIRV = FileSystem::ReadFileBinary("assets/shaders/frag.spv");

		// vertex shader
		VkShaderModule vertShaderModule = VK_NULL_HANDLE;
		VkShaderModuleCreateInfo vertShaderModuleCreateInfo{};
		vertShaderModuleCreateInfo.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		vertShaderModuleCreateInfo.pNext    = nullptr;
		vertShaderModuleCreateInfo.flags    = 0u;
		vertShaderModuleCreateInfo.codeSize = vertSPIRV.Size;
		vertShaderModuleCreateInfo.pCode    = (uint32*)vertSPIRV.Data;
		VK_CALL(vkCreateShaderModule(device, &vertShaderModuleCreateInfo, nullptr, &vertShaderModule));

		// fragment shader
		VkShaderModule fragShaderModule = VK_NULL_HANDLE;
		VkShaderModuleCreateInfo fragShaderModuleCreateInfo{};
		fragShaderModuleCreateInfo.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		fragShaderModuleCreateInfo.pNext    = nullptr;
		fragShaderModuleCreateInfo.flags    = 0u;
		fragShaderModuleCreateInfo.codeSize = fragSPIRV.Size;
		fragShaderModuleCreateInfo.pCode    = (uint32*)fragSPIRV.Data;
		VK_CALL(vkCreateShaderModule(device, &fragShaderModuleCreateInfo, nullptr, &fragShaderModule));

		VkPipelineShaderStageCreateInfo pipelineShaderStageCreateInfos[2];

		pipelineShaderStageCreateInfos[0].sType               = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		pipelineShaderStageCreateInfos[0].pNext               = nullptr;
		pipelineShaderStageCreateInfos[0].flags               = 0u;
		pipelineShaderStageCreateInfos[0].stage               = VK_SHADER_STAGE_VERTEX_BIT;
		pipelineShaderStageCreateInfos[0].module              = vertShaderModule;
		pipelineShaderStageCreateInfos[0].pName               = "main";
		pipelineShaderStageCreateInfos[0].pSpecializationInfo = nullptr;

		pipelineShaderStageCreateInfos[1].sType               = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		pipelineShaderStageCreateInfos[1].pNext               = nullptr;
		pipelineShaderStageCreateInfos[1].flags               = 0u;
		pipelineShaderStageCreateInfos[1].stage               = VK_SHADER_STAGE_FRAGMENT_BIT;
		pipelineShaderStageCreateInfos[1].module              = fragShaderModule;
		pipelineShaderStageCreateInfos[1].pName               = "main";
		pipelineShaderStageCreateInfos[1].pSpecializationInfo = nullptr;

		VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo{};
		pipelineVertexInputStateCreateInfo.sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		pipelineVertexInputStateCreateInfo.pNext                           = nullptr;
		pipelineVertexInputStateCreateInfo.flags                           = 0u;
		pipelineVertexInputStateCreateInfo.vertexBindingDescriptionCount   = 0u;
		pipelineVertexInputStateCreateInfo.pVertexBindingDescriptions      = nullptr;
		pipelineVertexInputStateCreateInfo.vertexAttributeDescriptionCount = 0u;
		pipelineVertexInputStateCreateInfo.pVertexAttributeDescriptions    = nullptr;

		VkPipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo{};
		pipelineInputAssemblyStateCreateInfo.sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		pipelineInputAssemblyStateCreateInfo.pNext                  = nullptr;
		pipelineInputAssemblyStateCreateInfo.flags                  = 0u;
		pipelineInputAssemblyStateCreateInfo.topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		pipelineInputAssemblyStateCreateInfo.primitiveRestartEnable = VK_FALSE;

		VkPipelineTessellationStateCreateInfo pipelineTessellationStateCreateInfo{};
		pipelineTessellationStateCreateInfo.sType              = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
		pipelineTessellationStateCreateInfo.pNext              = nullptr;
		pipelineTessellationStateCreateInfo.flags              = 0u;
		pipelineTessellationStateCreateInfo.patchControlPoints = 1u;

		VkViewport viewport{};
		viewport.x        = 0.0f;
		viewport.y        = 0.0f;
		viewport.width    = 0.0f;
		viewport.height   = 0.0f;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor{};
		scissor.offset = { 0u,0u };
		scissor.extent = { 0u,0u };

		VkPipelineViewportStateCreateInfo pipelineViewportStateCreateInfo{};
		pipelineViewportStateCreateInfo.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		pipelineViewportStateCreateInfo.pNext         = nullptr;
		pipelineViewportStateCreateInfo.flags         = 0u;
		pipelineViewportStateCreateInfo.viewportCount = 1u;
		pipelineViewportStateCreateInfo.pViewports    = &viewport;
		pipelineViewportStateCreateInfo.scissorCount  = 1u;
		pipelineViewportStateCreateInfo.pScissors     = &scissor;

		VkPipelineRasterizationStateCreateInfo pipelineRasterizationStateCreateInfo{};
		pipelineRasterizationStateCreateInfo.sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		pipelineRasterizationStateCreateInfo.pNext                   = nullptr;
		pipelineRasterizationStateCreateInfo.flags                   = 0u;
		pipelineRasterizationStateCreateInfo.depthClampEnable        = VK_FALSE;
		pipelineRasterizationStateCreateInfo.rasterizerDiscardEnable = VK_FALSE;
		pipelineRasterizationStateCreateInfo.polygonMode             = m_Specification.Wireframe ? VK_POLYGON_MODE_LINE : VK_POLYGON_MODE_FILL;
		pipelineRasterizationStateCreateInfo.cullMode                = VK_CULL_MODE_NONE;
		pipelineRasterizationStateCreateInfo.frontFace               = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		pipelineRasterizationStateCreateInfo.depthBiasEnable         = VK_FALSE;
		pipelineRasterizationStateCreateInfo.depthBiasConstantFactor = 0.0f;
		pipelineRasterizationStateCreateInfo.depthBiasClamp          = 0.0f;
		pipelineRasterizationStateCreateInfo.depthBiasSlopeFactor    = 0.0f;
		pipelineRasterizationStateCreateInfo.lineWidth               = 1.0f;

		VkPipelineMultisampleStateCreateInfo pipelineMultisampleStateCreateInfo{};
		pipelineMultisampleStateCreateInfo.sType                 = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		pipelineMultisampleStateCreateInfo.pNext                 = nullptr;
		pipelineMultisampleStateCreateInfo.flags                 = 0u;
		pipelineMultisampleStateCreateInfo.rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT;
		pipelineMultisampleStateCreateInfo.sampleShadingEnable   = VK_FALSE;
		pipelineMultisampleStateCreateInfo.minSampleShading      = 0.0f;
		pipelineMultisampleStateCreateInfo.pSampleMask           = nullptr;
		pipelineMultisampleStateCreateInfo.alphaToCoverageEnable = VK_FALSE;
		pipelineMultisampleStateCreateInfo.alphaToOneEnable      = VK_FALSE;

		VkPipelineDepthStencilStateCreateInfo pipelineDepthStencilStateCreateInfo{};
		pipelineDepthStencilStateCreateInfo.sType                 = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		pipelineDepthStencilStateCreateInfo.pNext                 = nullptr;
		pipelineDepthStencilStateCreateInfo.flags                 = 0u;
		pipelineDepthStencilStateCreateInfo.depthTestEnable       = VK_FALSE;
		pipelineDepthStencilStateCreateInfo.depthWriteEnable      = VK_FALSE;
		pipelineDepthStencilStateCreateInfo.depthCompareOp        = VK_COMPARE_OP_EQUAL;
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

		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		pushConstantRange.offset     = 0u;
		pushConstantRange.size       = sizeof(PushConstant);

		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
		pipelineLayoutCreateInfo.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutCreateInfo.pNext                  = nullptr;
		pipelineLayoutCreateInfo.flags                  = 0u;
		pipelineLayoutCreateInfo.setLayoutCount         = 0u;
		pipelineLayoutCreateInfo.pSetLayouts            = nullptr;
		pipelineLayoutCreateInfo.pushConstantRangeCount = 1u;
		pipelineLayoutCreateInfo.pPushConstantRanges    = &pushConstantRange;
		VK_CALL(vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, nullptr, &m_PipelineLayout));

		VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo{};
		graphicsPipelineCreateInfo.sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		graphicsPipelineCreateInfo.pNext               = nullptr;
		graphicsPipelineCreateInfo.flags               = 0u;
		graphicsPipelineCreateInfo.stageCount          = (uint32)std::size(pipelineShaderStageCreateInfos);
		graphicsPipelineCreateInfo.pStages             = pipelineShaderStageCreateInfos;
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
		graphicsPipelineCreateInfo.renderPass          = VulkanContext::Get().GetSwapchain().GetRenderPass();
		graphicsPipelineCreateInfo.subpass             = 0u;
		graphicsPipelineCreateInfo.basePipelineHandle  = VK_NULL_HANDLE;
		graphicsPipelineCreateInfo.basePipelineIndex   = 0;
		VK_CALL(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1u, &graphicsPipelineCreateInfo, nullptr, &m_Pipeline));

		vkDestroyShaderModule(device, vertShaderModule, nullptr);
		vkDestroyShaderModule(device, fragShaderModule, nullptr);
	}

	VulkanPipeline::~VulkanPipeline()
	{
		VkDevice device = VulkanContext::GetCurrentVulkanDevice();
		vkDestroyPipelineLayout(device, m_PipelineLayout, nullptr);
		vkDestroyPipeline(device, m_Pipeline, nullptr);
	}
}